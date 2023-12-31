#include "Webserver.hpp"

#include <cstring>			// strerror, memset
#include <cstdlib>			// exit
#include <cerrno>			// errno
#include <poll.h>			// poll, struct pollfd
#include <unistd.h>			// close
#include <sys/socket.h>		// accept, bind, listen, socket, getsockopt, getpeername, send, recv

#include "cgi.hpp"			// processCgiRequest
#include "http.hpp"			// parseHttpRequest, createHttpResponse, toString
#include "utils.hpp"		// toIpNum, toIpString, toInt
#include "Logger.hpp"		// Logger
#include "ConfigParser.hpp"	// ConfigParser

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
		LOG_ERROR << e.what();
		exit(1);
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
		LOG_ERROR << e.what();
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Initialization ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void Webserver::initListenSockets()
{
	set<Address> uniques = getUniqueAddresses(m_servers);

	set<Address>::iterator it;
	for (it = uniques.begin(); it != uniques.end(); it++) {
		int fd = createTcpListenSocket(*it);
		LOG_DEBUG << "new socket on " << toIpString(it->ip) << ":" << it->port;
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

	if (bytesRead == -1 || bytesRead == 0) {
		client.setHasDisconnected(true);
		return;
	}

	try {
		string bufferStr(buffer, bytesRead);
		if (!client.getRequestParsed()) client.parseHttpHeader(bufferStr);
		if (!client.getRequestIsReady()) client.parseHttpBody(bufferStr);

		if (client.getRequestIsReady()) {
			LOG_INFO
				<< BLUE << "HTTP Req>> Client[ID " << client.getId() << "] | "
				<< "Method[" << client.getRequest().method << "] "
				<< "URI[" << client.getRequest().uri << "]";

			HttpResponse res = processRequest(client.getRequest(), client);
			client.setResponse(res);
			client.reset();
		}
	}
	catch (const exception& e) {
		LOG_DEBUG << "handlePollIn exception: " << e.what();
		client.setResponse(createHttpResponse(400, DEFAULT_400));
	}
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Webserver::processRequest(HttpRequest req, Client& client)
{
	try {
		Server& server = routeRequest(req, client);

		LOG_INFO
			<< MAGENTA << "HTTP Route Client[ID " << client.getId() << "] | To: "
			<< server.getName();

		// Handle CGI
		if (req.uri.find(CGI_BIN) == 0) {
			return processCgiRequest(req, client, server);
		}

		return server.handleRequest(req);
	}
	catch (const exception& e) {
		LOG_DEBUG << "processRequest Exception: " << e.what();
		return createHttpResponse(400, DEFAULT_400);
	}
}

/* ---------------------------------------------------------------------------------------------- */
void Webserver::handlePollOut(Client& client)
{
	if (client.getResponseIsReady() == false) return;

	HttpResponse response = client.getResponse();

	LOG_INFO
		<< MAGENTA << "HTTP <<Res Client[ID " << client.getId() << "] | "
		<< "Status[" << response.statusCode << "]";

	string responseStr = toString(response);
	ssize_t sendBytes = send(client.getFd(), responseStr.c_str(), responseStr.size(), 0);
	if (sendBytes == -1 || sendBytes == 0) {
		client.setHasDisconnected(true);
	};
}

/* ---------------------------------------------------------------------------------------------- */
void Webserver::addClient(int listenFd)
{
	int clientFd = accept(listenFd, NULL, NULL);

	Client client(clientFd, getServerAddress(listenFd), getClientAddress(clientFd));
	m_clients.insert(std::make_pair(clientFd, client));

	m_pollFds.push_back(buildPollFd(clientFd, POLLIN | POLLOUT));

	LOG_INFO
		<< "Client[ID: " << client.getId() << "] connected on "
		<< toIpString(client.getServerIp()) << ":" << client.getPort();
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
				LOG_INFO << "Client[ID: " << client.getId() << "] disconnected";
			}
			else {
				LOG_INFO << "Client[ID: " << client.getId() << "] timed out";
			}
			removeIterators.push_back(it);
			removeFdFromPoll(client.getFd());
			close(client.getFd());
			LOG_DEBUG << "Closed fd " << client.getFd();
		}
	}

	for (size_t i = 0; i < removeIterators.size(); i++) {
		m_clients.erase(removeIterators[i]);
	}
	removeIterators.clear();
}

/* ---------------------------------------------------------------------------------------------- */
Server& Webserver::routeRequest(HttpRequest req, Client& client)
{
	if (req.header.find("Host") == req.header.end()) {
		throw runtime_error("No host header");
	}
	string host = req.header.find("Host")->second;

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
					? toIpNum(host.substr(0, colonPos))
					: toIpNum(host);
		int port = colonPos != string::npos
					? toInt(host.substr(colonPos + 1))
					: 80;

		if (port <= 0 || port > 65535) {
			throw runtime_error("port out of range");
		}

		for (size_t i = 0; i < m_servers.size(); i++) {
			Address addr = m_servers[i].getAddress();
			if (addr.ip == ip && addr.port == port) {
				return m_servers[i];
			}
		}
	}
	catch (...) {
	}

	// Default server resolution
	for (size_t i = 0; i < m_servers.size(); i++) {
		Address addr = m_servers[i].getAddress();
		if (client.getPort() == addr.port) {
			if (addr.ip == 0 || client.getServerIp() == addr.ip) {
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
			LOG_DEBUG << "Client fd " << fd << " removed from poll";
			m_pollFds.erase(m_pollFds.begin() + i);
			return;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */
set<Address> Webserver::getUniqueAddresses(vector<Server> servers)
{
	// if (checkForDuplicates(servers)) exit(1);

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
				if (it->ip == 0 && it->port == addr.port) break;
			}
			if (it == uniques.end()) uniques.insert(addr);
		}
	}
	return uniques;
}

/* ---------------------------------------------------------------------------------------------- */
bool Webserver::checkForDuplicates(vector<Server> servers)
{
	set<Address> dupes;
	for (size_t i = 0; i < servers.size(); i++) {
		if (!dupes.insert(servers[i].getAddress()).second) {
			LOG_ERROR << "Found duplicate!";
			return true;
		}
	}
	return false;
}

/* ---------------------------------------------------------------------------------------------- */
int Webserver::createTcpListenSocket(Address addr)
{
	int fd;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		throw runtime_error("socket() failed: " + string(strerror(errno)));
	}

	int sockopt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		static_cast<const void *>(&sockopt), sizeof(int)) == -1) {
		throw runtime_error("setsockopt() failed: " + string(strerror(errno)));
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(addr.port);
	serverAddr.sin_addr.s_addr = addr.ip;

	if (bind(fd, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0) {
		close(fd);
		throw runtime_error("bind() failed: " + string(strerror(errno)));
	}

	if (listen(fd, 10)) {
		throw runtime_error("listen() failed: " + string(strerror(errno)));
	}

	return fd;
}

/* ---------------------------------------------------------------------------------------------- */
PollFd Webserver::buildPollFd(int fd, short events)
{
	PollFd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	return pfd;
}

/* ---------------------------------------------------------------------------------------------- */
Address Webserver::getServerAddress(int fd)
{
	Sockaddr_in serverAddress;
	socklen_t addrLen = sizeof(serverAddress);
	if (getsockname(fd, reinterpret_cast<Sockaddr*>(&serverAddress), &addrLen) == -1) {
		throw runtime_error("getsockname() failed: " + string(strerror(errno)));
	}

	Address addr;
	addr.ip = serverAddress.sin_addr.s_addr;
	addr.port = ntohs(serverAddress.sin_port);

	return addr;
}

/* ---------------------------------------------------------------------------------------------- */
Address Webserver::getClientAddress(int fd)
{
	Sockaddr_in clientAddress;
	socklen_t addrLen = sizeof(clientAddress);
	if (getpeername(fd, reinterpret_cast<Sockaddr*>(&clientAddress), &addrLen) == -1) {
		throw runtime_error("getpeername() failed: " + string(strerror(errno)));
	}

	Address addr;
	addr.ip = clientAddress.sin_addr.s_addr;
	addr.port = ntohs(clientAddress.sin_port);

	return addr;
}
