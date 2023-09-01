#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <poll.h>
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
    Webserver(const vector<ServerConfig> serverConfigs);
    ~Webserver();

    void start();

private:
    // Initialization
    void setupServers(const vector<ServerConfig> configs);
    void initListenSockets();
    int initSocket(SocketAddress address);

    // Logic
    void mainloop();
	void handleClientPOLLIN(Client& client);
	void handleClientPOLLOUT(Client& client);

    HttpResponse processRequest(HttpRequest request);
    void addClient(int socketFd);
	void handleDisconnects();
    void clientStatusCheck(Client& client, int bytesRead);

    // Utility
    Client& getClientFromIdx(int idx);
    bool checkMethod(HttpRequest request);
    bool checkURL(HttpRequest request);
    bool checkVersion(HttpRequest request);
	bool checkHeaders(HttpRequest request);
    bool headerIsSupported(std::string header);
    void removeFdFromPoll(int fd);
    void printStatus();

    // Member Data
    set<int> m_listenSockets;
    vector<pollfd> m_pollFds;
    vector<Server> m_servers;
    map<int, Client> m_clients; // Key: Fd; Value: Client
};
