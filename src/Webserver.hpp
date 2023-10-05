#pragma once

#include "common.hpp"
#include "Server.hpp"
#include "Client.hpp"

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

	HttpResponse processRequest(HttpRequest req, Client& client);
	Server& routeRequest(HttpRequest req, Client& client);

	void addClient(int socketFd);
	void handleDisconnects();

	/* Utility */
	Client& getClientFromIdx(int idx);
	void removeFdFromPoll(int fd);
	set<Address> getUniqueAddresses(vector<Server> servers);
	int createTcpListenSocket(Address addr);
	PollFd buildPollFd(int fd, short events);
	Address getAddressFromFd(int fd);

	/* Member Data */
	size_t m_nbListenSockets;
	vector<struct pollfd> m_pollFds;
	vector<Server> m_servers;
	map<int, Client> m_clients; // Key: Fd; Value: Client
};
