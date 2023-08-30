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

#include "Server.hpp"
#include "Client.hpp"

#define RECV_SIZE 4096

#define CLIENT_TIMEOUT 60

class Server;
struct WebserverConfig{};
struct ServerConfig
{
    int port;
    std::string serverName;
    // std::map<int, std::string> errorPages;
    // size_t clientMaxBodySize;
    // std::vector<ConfigLocation> locations;
};
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
    int initSocket(int port);

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
    void removeFdFromPoll(int fd);
    void printStatus();

    // Member Data
    set<int> m_listenSockets;
    vector<pollfd> m_pollFds;
    vector<ServerConfig> m_serverConfigs;
    vector<Server> m_servers;
    map<int, Client> m_clients; // Key: Fd; Value: Client 
};