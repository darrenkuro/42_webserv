#pragma once

#include <cstdlib>
#include <queue>
#include <sys/time.h>

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"

#define TIMEOUT_TIME 30

class Client
{
public:
	Client();
	Client(int socketFd, in_addr host, int port);

	// Getters
	int getID();
	int getFd();
	int getPort();
	in_addr getHost();
	bool didTimeout();
	bool hasDisconnected();
	bool getResponseIsReady();
	bool getRequestIsReady();
	bool getRequestParsed();
	HttpResponse& getResponse();
	HttpRequest& getRequest();

	// Setters
	void setHasDisconnected(bool status);
	void setResponse(HttpResponse res);
	void setRequest(HttpRequest req);
	void setBytesExpected(int bytes);

	// Data processing
	void appendData(std::string buffer);
	void reset();

private:
	int m_id;
	int m_socketFd;
	int m_port;
	bool m_hasDisconnected;
	bool m_responseIsReady;
	bool m_requestIsReady;
	bool m_requestParsed;
	timeval m_lastEventTime;
	in_addr m_host;
	HttpResponse m_response;
	HttpRequest m_request;

	std::string m_dataRecved;
	int m_bytesRecved;
	int m_bytesExpected;
};
