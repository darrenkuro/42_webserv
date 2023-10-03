#pragma once

#include "infrastructure.hpp"

class Client
{
public:
	/* Constructor & Destructor */
	Client(int fd, Address addr);

	/* Getters */
	int getId() const;
	int getFd() const;
	int getPort() const;
	in_addr_t getIp() const;
	bool hasDisconnected() const ;
	bool getResponseIsReady() const;
	bool getRequestIsReady() const;
	bool getRequestParsed() const;
	bool didTimeout() const;
	HttpRequest& getRequest();
	HttpResponse& getResponse();

	/* Setters */
	void setHasDisconnected(bool status);
	void setRequest(HttpRequest req);
	void setResponse(HttpResponse res);
	void setBytesExpected(int bytes);

	/* Data Handling */
	void appendData(string buffer);
	void reset();

private:
	int m_id;
	int m_socketFd;
	bool m_hasDisconnected;
	bool m_responseIsReady;
	bool m_requestIsReady;
	bool m_requestParsed;
	time_t m_lastEventTime;
	Address m_address;
	HttpResponse m_response;
	HttpRequest m_request;

	string m_dataRecved;
	int m_bytesRecved;
	int m_bytesExpected;
};
