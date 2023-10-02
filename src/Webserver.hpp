#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <poll.h>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include "log.hpp"
#include "utils.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ConfigParser.hpp"

#define RECV_SIZE 4096
#define CLIENT_TIMEOUT 60

class Server;
extern bool g_running;

class Webserver
{
public:
	/* Constructors / Destructors */
	Webserver(std::string configPath);
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
	std::set<Address> getUniqueAddresses(std::vector<Server> servers);

	/* Member Data */
	size_t m_nbListenSockets;
	vector<struct pollfd> m_pollFds;
	vector<Server> m_servers;
	map<int, Client> m_clients; // Key: Fd; Value: Client
};
