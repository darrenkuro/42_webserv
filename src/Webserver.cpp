#include "Webserver.hpp"

/**
 * Webserver Construction/Deconstruction
*/
//------------------------------------------------------------------------------
Webserver::Webserver(const vector<ServerConfig> serverConfigs)
{
	log(INFO, "Webserver created");
	try {
		setupServers(serverConfigs);
	}
	catch (const std::exception& e) {
		log(ERROR, e.what());
		log(INFO, "Webserver setup failed");
	}
};

Webserver::~Webserver()
{
	for (size_t i = 0; i < m_pollFds.size(); i++) {
		close(m_pollFds[i].fd);
		log(DEBUG, "Closed fd %d", m_pollFds[i].fd);
	}
	log(INFO, "Webserver stopped");
};

void Webserver::start()
{
    try {
        initListenSockets();
		log(INFO, "Webserver started sucessfully. Now Listening...");
        mainloop();
    }
    catch(const std::exception& e) {
		log(ERROR, e.what());
    }
}

/**
 * Webserver Initialization Functions
*/
//------------------------------------------------------------------------------
void Webserver::setupServers(const vector<ServerConfig> configs)
{
	for (size_t i = 0; i < configs.size(); i++) {
		m_servers.push_back(Server(configs[i]));
	}
}

//------------------------------------------------------------------------------
void Webserver::initListenSockets()
{
	filterUniqueSockets();

	std::set<SocketAddress>::iterator it;
	for (it = m_listenSockets.begin(); it != m_listenSockets.end(); it++) {
		struct in_addr addr;
    	addr.s_addr = it->host;
		log(DEBUG, "Created listening socket on %s:%d", inet_ntoa(addr), it->port);
		int sockFd = initSocket(*it);
		struct pollfd poll = {sockFd, POLLIN, 0};
		m_pollFds.push_back(poll);
	}
}

//------------------------------------------------------------------------------
int Webserver::initSocket(SocketAddress address)
{
	int listenFd = createIPv4Socket();
	sockaddr_in serverAddr = createAddress(address);

	int sockopt = 1;
  	setsockopt (listenFd, SOL_SOCKET, SO_REUSEADDR, (const void *) &sockopt,
              sizeof (int));

	if (bind(listenFd, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		close(listenFd);
		throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
	}

	if (listen(listenFd, 10)) {
		throw std::runtime_error("listen() failed");
	}

	return listenFd;
}

/**
 * Webserver Logic Functions
*/
//------------------------------------------------------------------------------
void Webserver::mainloop()
{
	while (g_running) {
		int pollReady = poll(m_pollFds.data(), m_pollFds.size(), 1000);
		if (pollReady == -1) {
			throw std::runtime_error("poll() failed");
		}

		for (size_t i = m_listenSockets.size(); i < m_pollFds.size(); i++) {
			if (m_pollFds[i].revents & POLLIN) {
				handleClientPOLLIN(getClientFromIdx(i));
			}
			else if (m_pollFds[i].revents & POLLOUT) {
				handleClientPOLLOUT(getClientFromIdx(i));
			}
		}

		for (size_t i = 0; i < m_listenSockets.size(); i++) {
			if (m_pollFds[i].revents & POLLIN) {
				log(DEBUG, "New pollin on listenSocket");
				addClient(m_pollFds[i].fd);
			}
		}
		handleDisconnects();
	}
}

//------------------------------------------------------------------------------
void Webserver::handleClientPOLLIN(Client& client)
{
	char buffer[RECV_SIZE];
	ssize_t bytesRead = recv(client.getFd(), buffer, RECV_SIZE - 1, 0);

	if (bytesRead <= 0) {
		clientStatusCheck(client, bytesRead);
	}
	else {
		try {
			HttpRequest request = parseHttpRequest(std::string(buffer));
			logHttp(request, client.getID());
			HttpResponse response = processRequest(request, client);
			client.setResponse(response);
		}
		catch (...) {
			client.setResponse(createBasicResponse(400, DEFAULT_400_PATH));
		}
	}
}

//------------------------------------------------------------------------------
void Webserver::handleClientPOLLOUT(Client& client)
{
	if (client.getResponseIsReady() == false)
		return;

	HttpResponse response = client.getResponse();
	logHttp(response, client.getID());
	// Here turn reponse into string and send
	std::string responseStr = toString(response);
	send(client.getFd(), responseStr.c_str(), responseStr.size(), 0);
}

//------------------------------------------------------------------------------
HttpResponse Webserver::processRequest(HttpRequest request, Client& client)
{
	try {
		Server& server = routeRequest(request, client);
		log(INFO, "%sHTTP Route Client[ID %d]  |  To: %s%s", ORANGE, client.getID(),
			server.getName().c_str(), RESET);
		return server.handleRequest(request);
	}
	catch (std::exception& e) {
		log(DEBUG, e.what());
		return createBasicResponse(400, DEFAULT_400_PATH);
	}
}

//------------------------------------------------------------------------------
void Webserver::addClient(int socketFd)
{
	struct sockaddr_in serverAddress;
    socklen_t addrLen = sizeof(serverAddress);
	getsockname(socketFd, (struct sockaddr*)&serverAddress, &addrLen);

	int clientFd = accept(socketFd, NULL, NULL);
	pollfd clientPollData;
	clientPollData.fd = clientFd;
	clientPollData.events = POLLIN | POLLOUT;
	clientPollData.revents = 0;
	m_pollFds.push_back(clientPollData);
	Client client(clientFd, serverAddress.sin_addr, ntohs(serverAddress.sin_port));
	m_clients.insert(std::make_pair(clientFd, client));

	log(INFO, "Client[ID: %d] connected on %s:%d", client.getID(),
		inet_ntoa(client.getHost()), client.getPort());
	log(DEBUG, "Client fd %d added to poll", client.getFd());
}

//------------------------------------------------------------------------------
void Webserver::handleDisconnects()
{
	std::map<int, Client>::iterator it;
	std::vector<std::map<int, Client>::iterator> removeIterators;

	for (it = m_clients.begin(); it != m_clients.end(); it++) {
		Client client = it->second;
		if (client.hasDisconnected() || client.didTimeout()) {
			if (client.hasDisconnected())
				log(INFO, "Client[ID: %d] disconnected", client.getID());
			else
				log(INFO, "Client[ID: %d] timed out", client.getID());
			removeIterators.push_back(it);
			removeFdFromPoll(client.getFd());
			close(client.getFd());
			log(DEBUG, "Closed fd %d", client.getFd());
		}
	}

	for (size_t i = 0; i < removeIterators.size(); i++) {
		log(DEBUG, "Client[ID: %d] removed from client map", removeIterators[i]->second.getID());
		m_clients.erase(removeIterators[i]);
	}
	removeIterators.clear();
}

//------------------------------------------------------------------------------
void Webserver::clientStatusCheck(Client& client, int bytesRead)
{
	if (bytesRead == 0) {
		client.setHasDisconnected(true);
	}
	else {
		// Error or connection closed
	}
}

//------------------------------------------------------------------------------
Server& Webserver::routeRequest(HttpRequest request, Client& client)
{
	if (request.headers.find("Host") == request.headers.end()) {
		throw std::runtime_error("No host header");
	}
	std::string host = request.headers.find("Host")->second;

	// Host header domain resolution
	for (size_t i = 0; i < m_servers.size(); i++) {
		if (m_servers[i].getName() == host) {
			return m_servers[i];
		}
	}

	// Host header ip resolution
	size_t colonPos = host.find(':');
	std::string ip;
	std::string port;
	if (colonPos != std::string::npos) {
		ip = host.substr(0, colonPos);
		port = host.substr(colonPos + 1);
	}
	else {
		ip = host;
		port = "80";
	}
	if (validateIpAddress(ip) && validatePort(port)) {
		for (size_t i = 0; i < m_servers.size(); i++) {
			SocketAddress addr = m_servers[i].getAddress();
			if (addr.host == inet_addr(ip.c_str()) && addr.port == atoi(port.c_str())) {
				return m_servers[i];
			}
		}
	}

	// Default server resolution
	for (size_t i = 0; i < m_servers.size(); i++) {
		SocketAddress addr = m_servers[i].getAddress();
		if (client.getPort() == addr.port) {
			if (addr.host == 0 || client.getHost().s_addr == addr.host) {
				return m_servers[i];
			}
		}
	}

	throw std::runtime_error("Something went really wrong. should never reach this point");
}


/**
 * Webserver Utility Functions
*/
//------------------------------------------------------------------------------
void Webserver::filterUniqueSockets()
{
	for (size_t i = 0; i < m_servers.size(); i++) {
		m_listenSockets.insert(m_servers[i].getAddress());
	}

	vector<std::set<SocketAddress>::iterator> removeIterators;
	std::set<SocketAddress>::iterator it;
	std::set<SocketAddress>::iterator it2;
	for (it = m_listenSockets.begin(); it != m_listenSockets.end(); it++) {
		if (it->host == 0) {
			for (it2 = m_listenSockets.begin(); it2 != m_listenSockets.end(); it2++) {
				if (it2->port == it->port && it2 != it) {
					removeIterators.push_back(it2);
				}
			}
		}
	}

	for (size_t i = 0; i < removeIterators.size(); i++) {
		m_listenSockets.erase(removeIterators[i]);
	}
}

//------------------------------------------------------------------------------
Client& Webserver::getClientFromIdx(int idx)
{
	std::map<int, Client>::iterator it;
	it = m_clients.find(m_pollFds[idx].fd);
	Client& client = it->second;
	return client;
}

//------------------------------------------------------------------------------
void Webserver::removeFdFromPoll(int fd)
{
	for (size_t i = 0; i < m_pollFds.size(); i++) {
		if (m_pollFds[i].fd == fd) {
			log(DEBUG, "Client fd %d removed from poll", fd);
			m_pollFds.erase(m_pollFds.begin() + i);
			return;
		}
	}
}

//------------------------------------------------------------------------------
bool Webserver::headerIsSupported(std::string header)
{
	for (int i = 0; i < 1; i++) {
		if (header == "Host") {
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
void Webserver::printStatus()
{

}
