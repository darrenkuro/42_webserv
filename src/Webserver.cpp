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
		log(INFO, "Webserver started sucessfully");
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
    for (size_t i = 0; i < m_servers.size(); i++) {
		m_listenSockets.insert(initSocket(m_servers[i].getPort()));
	}
	std::set<int>::iterator it;
	for (it = m_listenSockets.begin(); it != m_listenSockets.end(); it++) {
		log(DEBUG, "Listen fd %d added to poll", *it);
		struct pollfd poll = {*it, POLLIN, 0};
		m_pollFds.push_back(poll);
	}
}

//------------------------------------------------------------------------------
int Webserver::initSocket(int port)
{
	int listenFd = createIPv4Socket();
	sockaddr_in serverAddr = createAddress(port);

	if (bind(listenFd, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		throw std::runtime_error("bind() failed");
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
		HttpRequest request = parseHttpRequest(std::string(buffer));
		logHttp(request, client.getID());
		HttpResponse response = processRequest(request);
		client.setResponse(response);
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
}

//------------------------------------------------------------------------------
HttpResponse Webserver::processRequest(HttpRequest request)
{
	(void)request;
	HttpResponse dummyResponse;
	dummyResponse.version = "HTTP/1.1";
	dummyResponse.statusCode = 200;
	dummyResponse.statusText = "OK";
	return dummyResponse;
}

//------------------------------------------------------------------------------
void Webserver::addClient(int socketFd)
{
	int clientFd = accept(socketFd, NULL, NULL);
	pollfd clientPollData;
	clientPollData.fd = clientFd;
	clientPollData.events = POLLIN | POLLOUT;
	clientPollData.revents = 0;
	m_pollFds.push_back(clientPollData);
	Client client(clientFd);
	m_clients.insert(std::make_pair(clientFd, client));
	log(INFO, "Client[ID: %d] connected", client.getID());
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