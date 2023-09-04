#include "Client.hpp"

Client::Client(int socketFd, in_addr host, int port)
{
    static int clientID = 1;
    m_socketFd = socketFd;
    m_responseIsReady = false;
    m_port = port;
    m_host = host;
    m_hasDisconnected = false;
    m_id = clientID;
    gettimeofday(&m_lastEventTime, NULL);
    clientID++;
}

void Client::setResponse(HttpResponse response)
{
    m_response = response;
    gettimeofday(&m_lastEventTime, NULL);
    m_responseIsReady = true;
}

HttpResponse Client::getResponse()
{
    m_responseIsReady = false;
    return m_response;
}

int Client::getID() { return m_id; }

int Client::getFd() { return m_socketFd; }

int Client::getPort() { return m_port; }

in_addr Client::getHost() { return m_host; }

bool Client::hasDisconnected() { return m_hasDisconnected; }

bool Client::getResponseIsReady() { return m_responseIsReady; }

void Client::setHasDisconnected(bool status) { m_hasDisconnected = status; }

bool Client::didTimeout()
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    long long timeDiffS = (currentTime.tv_sec - m_lastEventTime.tv_sec);
    return (timeDiffS > TIMEOUT_TIME);
}
