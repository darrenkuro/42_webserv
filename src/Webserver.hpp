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
using std::vector;
using std::cout;
using std::endl;
using std::cerr;
using std::set;
using std::map;


class Webserver
{
public:
	Webserver(std::string configPath);
	~Webserver();

	void start(void);

private:
	// Initialization
	void initListenSockets();

	// Logic
	void mainloop();
	void handlePOLLIN(Client& client);
	void handlePOLLOUT(Client& client);

	HttpResponse processRequest(HttpRequest request, Client& client);
	void addClient(int socketFd);
	void handleDisconnects();
	Server& routeRequest(HttpRequest request, Client& client);

	// Utility
	Client& getClientFromIdx(int idx);
	void removeFdFromPoll(int fd);
	std::set<Address> getUniqueAddresses(std::vector<Server> servers);

	// Member Data
	size_t m_nbListenSockets;
	vector<pollfd> m_pollFds;
	vector<Server> m_servers;
	map<int, Client> m_clients; // Key: Fd; Value: Client
};
