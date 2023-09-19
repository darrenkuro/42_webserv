#include "Client.hpp"

Client::Client(int socketFd, in_addr host, int port)
{
	static int clientID = 1;
	m_socketFd = socketFd;
	m_port = port;
	m_host = host;
	m_responseIsReady = false;
	m_hasDisconnected = false;
	m_id = clientID;
	gettimeofday(&m_lastEventTime, NULL);
	reset();
	clientID++;
}

void Client::setResponse(HttpResponse res)
{
	m_response = res;
	gettimeofday(&m_lastEventTime, NULL);
	m_responseIsReady = true;
	m_requestParsed = false;
}

void Client::setRequest(HttpRequest req)
{
	m_request = req;
	m_requestParsed = true;
}

HttpResponse& Client::getResponse()
{
	m_responseIsReady = false;
	return m_response;
}

HttpRequest& Client::getRequest()
{
	return m_request;
}

int Client::getID() { return m_id; }

int Client::getFd() { return m_socketFd; }

int Client::getPort() { return m_port; }

in_addr Client::getHost() { return m_host; }

bool Client::hasDisconnected() { return m_hasDisconnected; }

bool Client::getResponseIsReady() { return m_responseIsReady; }

bool Client::getRequestIsReady() { return m_requestIsReady; }

bool Client::getRequestParsed() { return m_requestParsed; }

void Client::setHasDisconnected(bool status) { m_hasDisconnected = status; }

void Client::setBytesExpected(int bytes)
{
	m_bytesExpected = bytes;
	m_requestIsReady = false;
}

void Client::reset()
{
	m_bytesExpected = 0;
	m_bytesRecved = 0;
	m_requestIsReady = true;
	m_requestParsed = false;
}

bool Client::didTimeout()
{
	timeval currentTime;
	gettimeofday(&currentTime, NULL);

	long long timeDiffS = (currentTime.tv_sec - m_lastEventTime.tv_sec);
	return (timeDiffS > TIMEOUT_TIME);
}

void Client::appendData(std::string buffer)
{
	//m_request = appendBody(m_request, buffer);
	m_request.body.append(buffer);
	m_bytesRecved += buffer.length();
	if (m_bytesRecved >= m_bytesExpected)
		m_requestIsReady = true;
}
