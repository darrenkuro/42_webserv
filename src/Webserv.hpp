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
#include "HttpParser.hpp"

#define RECV_SIZE 1000

struct WebservConfig
{
    std::string serverName;
    int port;
};

typedef std::vector<struct pollfd>::iterator pollIt;

class Webserv
{
public:
    Webserv();
    Webserv(const WebservConfig config);
    Webserv(const Webserv &rhs);
    Webserv &operator=(const Webserv &rhs);

    ~Webserv();

    void launch();
    void stop();

private:
    void initSocket();
    void polling();
    void connectClient(int clientFd);
    void disconnectClient(pollIt &client);

    const WebservConfig m_config;
    int m_sockfd;
    int m_pid;
    struct sockaddr_in m_serverAddr;
    std::vector<struct pollfd> m_pollFds;
};