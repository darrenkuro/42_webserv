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
    Client(int socketFd);

    // Getters
    int getID();
    int getFd();
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
    HttpResponse m_response;
};