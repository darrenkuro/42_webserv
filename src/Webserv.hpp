#pragma once

#include <string>
#include <netinet/in.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include "HttpRequest.hpp"

#define RECV_SIZE 10000

struct WebservConfig
{
    std::string serverName;
    int port;
};

typedef std::vector<pollfd>::iterator pollIt;

class Webserv
{
public:
    Webserv();
    Webserv(const WebservConfig config);
    Webserv(const Webserv &rhs);
    Webserv &operator=(const Webserv &rhs);

    ~Webserv();

    void launch();
    void mainloop();
    void stop();

private:
    void initSocket();
    void connectNewClient();

    void clientErrorHandling(ssize_t bytesRead, int clientIdx);
    void processClientEvents(int clientIdx, char* buffer);

    void cleanupDisconnClients();


    const WebservConfig m_config;
    int m_listeningSock;
    int m_pid;
    sockaddr_in m_serverAddr;
    std::vector<pollfd> m_pollFds;
    std::vector<int> m_disconnClientIdxs;
};