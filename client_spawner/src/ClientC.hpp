#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

class ClientC
{
public:
    ClientC();
    ~ClientC();
    void connect(std::string ipAddress, int port);
    void disconnect();
    void sendRequest(std::string request);
private:
    int m_socketFd;
};
