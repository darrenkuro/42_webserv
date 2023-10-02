#include "Webserver.hpp"
#include <arpa/inet.h>

using std::string;
using std::set;
using std::map;
using std::vector;
using std::exception;
using std::runtime_error;

///////////////////////////////////////////////
//  Webserver Construction & Deconstruction  //
///////////////////////////////////////////////

/* --------------------------------------------------------------------------------------------- */
Webserver::Webserver(string configPath)
{
	try {
		ConfigParser parser;
		vector<ServerConfig> configs = parser.parse(configPath);
		for (size_t i = 0; i < configs.size(); i++) {
			m_servers.push_back(Server(configs[i]));
		}
	}
	catch (const exception& e) {
		log(ERROR, e.what());
	}
};

Webserver::~Webserver()
{
	for (size_t i = 0; i < m_pollFds.size(); i++) {
		close(m_pollFds[i].fd);
	}
};

void Webserver::start(void)
{
	try {
		initListenSockets();
		mainloop();
	}
	catch(const exception& e) {
		log(ERROR, e.what());
	}
}

//////////////////////////////////////////
//  Webserver Initialization Functions  //
//////////////////////////////////////////

/* --------------------------------------------------------------------------------------------- */
void Webserver::initListenSockets()
{
	set<Address> uniques = getUniqueAddresses(m_servers);

	set<Address>::iterator it;
	for (it = uniques.begin(); it != uniques.end(); it++) {
		int fd = createTcpListenSocket(*it);
		log(DEBUG, "new socket on %s:%d", toIPString(it->ip).c_str(), it->port);
		m_pollFds.push_back(buildPollFd(fd, POLLIN));
	}
	m_nbListenSockets = uniques.size();
}

/////////////////////////////////
//  Webserver Logic Functions  //
/////////////////////////////////

/* --------------------------------------------------------------------------------------------- */
void Webserver::mainloop()
{
	while (g_running) {
		int pollReady = poll(m_pollFds.data(), m_pollFds.size(), 1000);
		if (pollReady == -1) {
			throw runtime_error("poll() failed");
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

/* --------------------------------------------------------------------------------------------- */
void Webserver::handlePOLLIN(Client& client)
{
	char buffer[RECV_SIZE];
	ssize_t bytesRead = recv(client.getFd(), buffer, RECV_SIZE, 0);
	string bufferStr(buffer, bytesRead);

	if (bytesRead == -1)
		throw runtime_error("recv() failed");
	if (bytesRead == 0) {
		client.setHasDisconnected(true);
		return;
	}

	try {
		HttpRequest req = parseHttpRequest(string(buffer));
		client.setResponse(processRequest(req, client));
		logHttp(req, client.getID());
	}
	catch (...) {
		client.setResponse(createBasicResponse(400, DEFAULT_400_PATH));
	}
}

/* --------------------------------------------------------------------------------------------- */
HttpResponse Webserver::processRequest(HttpRequest request, Client& client)
{
	Server& server = routeRequest(request, client);
	log(INFO, "%sHTTP Route Client[ID %d]  |  To: %s%s", ORANGE, client.getID(),
		server.getName().c_str(), RESET);
	return server.handleRequest(request);
}

/* --------------------------------------------------------------------------------------------- */
void Webserver::handlePOLLOUT(Client& client)
{
	if (client.getResponseIsReady() == false)
		return;

	HttpResponse response = client.getResponse();
	logHttp(response, client.getID());
	string responseStr = toString(response);
	send(client.getFd(), responseStr.c_str(), responseStr.size(), 0);
}

/* --------------------------------------------------------------------------------------------- */
void Webserver::addClient(int listenFd)
{
	int clientFd = accept(listenFd, NULL, NULL);

	Client client(clientFd, getAddressFromFd(listenFd));
	m_clients.insert(std::make_pair(clientFd, client));

	m_pollFds.push_back(buildPollFd(clientFd, POLLIN | POLLOUT));

	log(INFO, "Port = %d", client.getPort());

	log(INFO, "Client[ID: %d] connected on %s:%d", client.getID(),
		toIPString(client.getIP()).c_str(), client.getPort());
}

/* --------------------------------------------------------------------------------------------- */
void Webserver::handleDisconnects(void)
{
	map<int, Client>::iterator it;
	vector<map<int, Client>::iterator> removeIterators;

	for (it = m_clients.begin(); it != m_clients.end(); it++) {
		Client client = it->second;
		if (client.hasDisconnected() || client.didTimeout()) {
			if (client.hasDisconnected()) {
				log(INFO, "Client[ID: %d] disconnected", client.getID());
			}
			else {
				log(INFO, "Client[ID: %d] timed out", client.getID());
			}
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

/* --------------------------------------------------------------------------------------------- */
Server& Webserver::routeRequest(HttpRequest request, Client& client)
{
	if (request.header.find("Host") == request.header.end()) {
		throw runtime_error("No host header");
	}
	string host = request.header.find("Host")->second;

	// Host header domain resolution
	for (size_t i = 0; i < m_servers.size(); i++) {
		if (m_servers[i].getName() == host) {
			return m_servers[i];
		}
	}

	// Host header ip resolution
	size_t colonPos = host.find(':');
	in_addr_t ip = colonPos != string::npos
				? toIPv4(host.substr(0, colonPos))
				: toIPv4(host);
	int port = colonPos != string::npos
				? toInt(host.substr(colonPos + 1))
				: 80;
	if (port <= 0 || port > 65535) {
		throw exception();
	}
	for (size_t i = 0; i < m_servers.size(); i++) {
		Address addr = m_servers[i].getAddress();
		if (addr.ip == ip && addr.port == port) {
			return m_servers[i];
		}
	}

	// Default server resolution
	for (size_t i = 0; i < m_servers.size(); i++) {
		Address addr = m_servers[i].getAddress();
		if (client.getPort() == addr.port) {
			if (addr.ip == 0 || client.getIP() == addr.ip) {
				return m_servers[i];
			}
		}
	}

	throw runtime_error("Something went really wrong when routing server!");
}

///////////////////////////////////
//  Webserver Utility Functions  //
///////////////////////////////////

/* --------------------------------------------------------------------------------------------- */
Client& Webserver::getClientFromIdx(int idx)
{
	map<int, Client>::iterator it;
	it = m_clients.find(m_pollFds[idx].fd);
	Client& client = it->second;
	return client;
}

/* --------------------------------------------------------------------------------------------- */
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

/* --------------------------------------------------------------------------------------------- */
set<Address> Webserver::getUniqueAddresses(vector<Server> servers)
{
	set<Address> uniques;
	for (size_t i = 0; i < servers.size(); i++) {
		if (servers[i].getAddress().ip == 0) {
			uniques.insert(servers[i].getAddress());
		}
	}

	for (size_t i = 0; i < servers.size(); i++) {
		Address addr = servers[i].getAddress();
		if (addr.ip != 0) {
			set<Address>::iterator it;
			for (it = uniques.begin(); it != uniques.end(); it++) {
				if (it->ip == 0 && it->port == addr.port)
					break;
			}
			if (it == uniques.end())
				uniques.insert(addr);
		}
	}
	return uniques;
}
