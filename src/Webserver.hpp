#pragma once

#include <poll.h>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include "infrastructure.hpp"
#include "utils.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ConfigParser.hpp"

class Server;
extern bool g_running;

class Webserver
{
public:
	/* Constructors & Destructors */
	Webserver(string configPath);
	~Webserver();

	/* Start */
	void start(void);

private:
	/* Initialization */
	void initListenSockets();

	/* Logic */
	void mainloop();
	void handlePollIn(Client& client);
	void handlePollOut(Client& client);

	HttpResponse processRequest(HttpRequest request, Client& client);
	Server& routeRequest(HttpRequest request, Client& client);

	void addClient(int socketFd);
	void handleDisconnects();

	/* Utility */
	Client& getClientFromIdx(int idx);
	void removeFdFromPoll(int fd);
	set<Address> getUniqueAddresses(vector<Server> servers);
	int createTcpListenSocket(Address addr);
	PollFd buildPollFd(int fd, short events);

	/* Member Data */
	size_t m_nbListenSockets;
	vector<struct pollfd> m_pollFds;
	vector<Server> m_servers;
	map<int, Client> m_clients; // Key: Fd; Value: Client
};
