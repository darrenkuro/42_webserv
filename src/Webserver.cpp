#include "Webserver.hpp"

/**
 * Webserver Construction/Deconstruction/Start
 * ------------------------------------------------------------------------------
*/
Webserver::Webserver(std::string configPath)
{
	try {
		ConfigParser parser;
		std::vector<ServerConfig> configs = parser.parse(configPath);
		for (size_t i = 0; i < configs.size(); i++) {
			m_servers.push_back(Server(configs[i]));
		}
	}
	catch (const std::exception& e) {
		log(ERROR, e.what());
	}
};

Webserver::~Webserver()
{
	for (size_t i = 0; i < m_pollFds.size(); i++) {
		close(m_pollFds[i].fd);
	}
};

void Webserver::start()
{
    try {
        initListenSockets();
        mainloop();
    }
    catch(const std::exception& e) {
		log(ERROR, e.what());
    }
}

/**
 * Webserver Initialization Functions
 * ------------------------------------------------------------------------------
*/
void Webserver::initListenSockets()
{
	std::set<Address> uniques = getUniqueAddresses(m_servers);

	std::set<Address>::iterator it;
	for (it = uniques.begin(); it != uniques.end(); it++) {
		int fd = createTcpListenSocket(*it);
		m_pollFds.push_back(buildPollFd(fd, POLLIN));
	}
	m_nbListenSockets = uniques.size();
}

/**
 * Webserver Logic Functions
 * ------------------------------------------------------------------------------
*/
void Webserver::mainloop()
{
	while (g_running) {
		int pollReady = poll(m_pollFds.data(), m_pollFds.size(), 1000);
		if (pollReady == -1) {
			throw std::runtime_error("poll() failed");
		}

		for (size_t i = m_nbListenSockets; i < m_pollFds.size(); i++) {
			if (m_pollFds[i].revents & POLLIN) {
				handlePOLLIN(getClientFromIdx(i));
			}
			else if (m_pollFds[i].revents & POLLOUT) {
				handlePOLLOUT(getClientFromIdx(i));
			}
		}

		for (size_t i = 0; i < m_nbListenSockets; i++) {
			if (m_pollFds[i].revents & POLLIN) {
				addClient(m_pollFds[i].fd);
			}
		}
		handleDisconnects();
	}
}

//------------------------------------------------------------------------------
void Webserver::handlePOLLIN(Client& client)
{
	char buffer[RECV_SIZE];
	ssize_t bytesRead = recv(client.getFd(), buffer, RECV_SIZE - 1, 0);

	if (bytesRead == -1)
		throw std::runtime_error("recv() failed");
	if (bytesRead == 0) {
		client.setHasDisconnected(true);
		return;
	}

	try {
		HttpRequest req = parseHttpRequest(std::string(buffer));
		client.setResponse(processRequest(req, client));
		logHttp(req, client.getID());
	}
	catch (...) {
		client.setResponse(createBasicResponse(400, DEFAULT_400_PATH));
	}
}

//------------------------------------------------------------------------------
HttpResponse Webserver::processRequest(HttpRequest request, Client& client)
{
	Server& server = routeRequest(request, client);
	log(INFO, "%sHTTP Route Client[ID %d]  |  To: %s%s", ORANGE, client.getID(),
		server.getName().c_str(), RESET);
	return server.handleRequest(request);
}

//------------------------------------------------------------------------------
void Webserver::handlePOLLOUT(Client& client)
{
	if (client.getResponseIsReady() == false)
		return;

	HttpResponse response = client.getResponse();
	logHttp(response, client.getID());
	std::string responseStr = toString(response);
	send(client.getFd(), responseStr.c_str(), responseStr.size(), 0);
}

//------------------------------------------------------------------------------
void Webserver::addClient(int listenFd)
{
	int clientFd = accept(listenFd, NULL, NULL);

	Client client(clientFd, getAddressFromFd(listenFd));
	m_clients[clientFd] = client;

	m_pollFds.push_back(buildPollFd(clientFd, POLLIN | POLLOUT));

	log(INFO, "Client[ID: %d] connected on %s:%d", client.getID(),
		inet_ntoa(client.getHost()), client.getPort());
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
		m_clients.erase(removeIterators[i]);
	}
	removeIterators.clear();
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
void Webserver::printStatus()
{

}
