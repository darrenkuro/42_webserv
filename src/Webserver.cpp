#include "Webserver.hpp"
#include <arpa/inet.h>

/* ============================================================================================== */
/*                                                                                                */
/*                                 Webserver Class Implementation                                 */
/*                                                                                                */
/* ============================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Constructors & Destructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
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

/* ---------------------------------------------------------------------------------------------- */
Webserver::~Webserver()
{
	for (size_t i = 0; i < m_pollFds.size(); i++) {
		close(m_pollFds[i].fd);
	}
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Start ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Initialization ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Logic ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void Webserver::mainloop()
{
	while (g_running) {
		int pollReady = poll(m_pollFds.data(), m_pollFds.size(), 1000);
		if (pollReady == -1) {
			throw runtime_error("poll() failed");
		}

		for (size_t i = m_nbListenSockets; i < m_pollFds.size(); i++) {
			if (m_pollFds[i].revents & POLLIN) {
				handlePollIn(getClientFromIdx(i));
			}
			else if (m_pollFds[i].revents & POLLOUT) {
				handlePollOut(getClientFromIdx(i));
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

/* ---------------------------------------------------------------------------------------------- */
void Webserver::handlePollIn(Client& client)
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

	// HTTP request is longer than buffer
	if (!client.getRequestParsed() && bytesRead == RECV_SIZE) {
		client.setResponse(createBasicResponse(400, DEFAULT_400));
		return;
	}

	try {
		if (!client.getRequestParsed()) {
			client.setRequest(parseHttpRequest(bufferStr));

			// Check Content-Length and set expecting bytes
			StringMap::iterator it = client.getRequest().header.find("Content-Length");
			if (it != client.getRequest().header.end()) {
				client.setBytesExpected(toInt(it->second));
			}
		}

		if (!client.getRequestIsReady()) client.appendData(bufferStr);

		if (client.getRequestIsReady()) {
			log(client.getRequest(), client.getId());
			HttpResponse res = processRequest(client.getRequest(), client);
			client.setResponse(res);
			//client.reset();
		}
	}
	catch (...) {
		client.setResponse(createBasicResponse(400, DEFAULT_400));
	}
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Webserver::processRequest(HttpRequest request, Client& client)
{
	try {
		Server& server = routeRequest(request, client);
		log(INFO, "%sHTTP Route Client[ID %d]  |  To: %s%s", ORANGE, client.getId(),
			server.getName().c_str(), RESET);
		return server.handleRequest(request);
	}
	catch (const exception& e) {
		log(DEBUG, e.what());
		return createBasicResponse(400, DEFAULT_400);
	}
}

/* ---------------------------------------------------------------------------------------------- */
HttpRequest Webserver::parseHttpRequest(string content)
{
	HttpRequest req;

	parseRequestPart(" ", req.method, content);
	parseRequestPart(" ", req.uri, content);
	parseRequestPart("\r\n", req.version, content);
	while (content.find("\r\n") != 0) {
		parseRequestHeader(req.header, content);
	}

	// Remove the blank line after the headers
	content.erase(content.begin(), content.begin() + 2);

	// GET & DELETE usually contains no relavant body
	return req;
}

/* ---------------------------------------------------------------------------------------------- */
void Webserver::handlePollOut(Client& client)
{
	if (client.getResponseIsReady() == false)
		return;

	HttpResponse response = client.getResponse();
	log(response, client.getId());
	string responseStr = toString(response);
	send(client.getFd(), responseStr.c_str(), responseStr.size(), 0);
}

/* ---------------------------------------------------------------------------------------------- */
void Webserver::addClient(int listenFd)
{
	int clientFd = accept(listenFd, NULL, NULL);

	Client client(clientFd, getAddressFromFd(listenFd));
	m_clients.insert(std::make_pair(clientFd, client));

	m_pollFds.push_back(buildPollFd(clientFd, POLLIN | POLLOUT));

	log(INFO, "Port = %d", client.getPort());

	log(INFO, "Client[ID: %d] connected on %s:%d", client.getId(),
		toIPString(client.getIp()).c_str(), client.getPort());
}

/* ---------------------------------------------------------------------------------------------- */
void Webserver::handleDisconnects(void)
{
	map<int, Client>::iterator it;
	vector<map<int, Client>::iterator> removeIterators;

	for (it = m_clients.begin(); it != m_clients.end(); it++) {
		Client client = it->second;
		if (client.hasDisconnected() || client.didTimeout()) {
			if (client.hasDisconnected()) {
				log(INFO, "Client[ID: %d] disconnected", client.getId());
			}
			else {
				log(INFO, "Client[ID: %d] timed out", client.getId());
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

/* ---------------------------------------------------------------------------------------------- */
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

	try {
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
	}
	catch (...) { }

	// Default server resolution
	for (size_t i = 0; i < m_servers.size(); i++) {
		Address addr = m_servers[i].getAddress();
		if (client.getPort() == addr.port) {
			if (addr.ip == 0 || client.getIp() == addr.ip) {
				return m_servers[i];
			}
		}
	}

	throw runtime_error("Something went really wrong when routing server!");
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Utility ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
Client& Webserver::getClientFromIdx(int idx)
{
	map<int, Client>::iterator it;
	it = m_clients.find(m_pollFds[idx].fd);
	Client& client = it->second;
	return client;
}

/* ---------------------------------------------------------------------------------------------- */
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

/* ---------------------------------------------------------------------------------------------- */
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

/* ---------------------------------------------------------------------------------------------- */
void Webserver::parseRequestPart(const string& sep, string& field, string& content)
{
	size_t pos = content.find(sep);
	if (pos == string::npos) {
		throw exception();
	}

	field = content.substr(0, pos);
	content.erase(content.begin(), content.begin() + pos + sep.length());

	// Check if there's more than one separator (invalid request)
	if (content.find(sep) == 0) {
		throw exception();
	}
}

/* ---------------------------------------------------------------------------------------------- */
void Webserver::parseRequestHeader(StringMap& header, string& content)
{
	size_t colonPos = content.find(":");
	if (colonPos == string::npos) {
		throw exception();
	}

	string key = content.substr(0, colonPos);
	content.erase(content.begin(), content.begin() + colonPos + 1);

	// Handle leading white spaces before value
	size_t valuePos = content.find_first_not_of(" \t");
	if (valuePos == string::npos) {
		throw exception();
	}
	content.erase(content.begin(), content.begin() + valuePos);

	size_t crlfPos = content.find("\r\n");
	if (crlfPos == string::npos) {
		throw exception();
	}
	string value = content.substr(0, crlfPos);
	content.erase(content.begin(), content.begin() + crlfPos + 2);

	header[key] = value;
}

/* ---------------------------------------------------------------------------------------------- */
int Webserver::createTcpListenSocket(Address addr)
{
	int fd;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw runtime_error("socket() failed" + string(strerror(errno)));

	int sockopt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &sockopt, sizeof (int)) == -1)
		throw runtime_error("setsockopt() failed: " + string(strerror(errno)));

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(addr.port);
	serverAddr.sin_addr.s_addr = addr.ip;

	if (bind(fd, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		close(fd);
		throw runtime_error("bind() failed: " + string(strerror(errno)));
	}

	if (listen(fd, 100))
		throw runtime_error("listen() failed" + string(strerror(errno)));

	return fd;
}

PollFd Webserver::buildPollFd(int fd, short events)
{
	PollFd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	return pfd;
}
