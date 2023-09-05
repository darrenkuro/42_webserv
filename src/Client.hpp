#pragma once

#include <cstdlib>
#include <queue>
#include <sys/time.h>

#include "HttpResponse.hpp"

#define TIMEOUT_TIME 30

class Client
{
public:
    Client();
    Client(int socketFd, in_addr host, int port);
    Client(int fd, Address addr);

    // Getters
    int getID();
    int getFd();
    int getPort();
    in_addr getHost();
    bool didTimeout();
    bool hasDisconnected();
    bool getResponseIsReady();
    HttpResponse getResponse();

    // Setters
    void setHasDisconnected(bool status);
    void setResponse(HttpResponse response);

private:
    int m_id;
    int m_socketFd;
    bool m_hasDisconnected;
    bool m_responseIsReady;
    timeval m_lastEventTime;
    int m_port;
    in_addr m_host;
    HttpResponse m_response;
};