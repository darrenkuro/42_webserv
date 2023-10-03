#pragma once

#include <cstdlib>
#include <queue>
#include <sys/time.h>
#include "global.hpp"
#include "utils.hpp"

#define TIMEOUT_TIME 30

class Client
{
public:
	/* Constructor & Destructor */
	Client(int fd, Address addr);

	/* Getters */
	int getId();
	int getFd();
	int getPort();
	in_addr_t getIp();
	bool didTimeout();
	bool hasDisconnected();
	bool getResponseIsReady();
	bool getRequestIsReady();
	bool getRequestParsed();
	HttpResponse& getResponse();
	HttpRequest& getRequest();

	/* Setters */
	void setHasDisconnected(bool status);
	void setResponse(HttpResponse res);
	void setRequest(HttpRequest req);
	void setBytesExpected(int bytes);

	/* Data processing */
	void appendData(std::string buffer);
	void reset();

private:
	int m_id;
	int m_socketFd;
	bool m_hasDisconnected;
	bool m_responseIsReady;
	bool m_requestIsReady;
	bool m_requestParsed;
	timeval m_lastEventTime;
	Address m_address;
	HttpResponse m_response;
	HttpRequest m_request;

	string m_dataRecved;
	int m_bytesRecved;
	int m_bytesExpected;
};
