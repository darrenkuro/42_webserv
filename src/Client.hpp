#pragma once

#include "common.hpp"

class Client
{
public:
	/* Constructor & Destructor */
	Client(int fd, Address serverAddr, Address ClientAddr);

	/* Getters */
	int getId() const;
	int getFd() const;
	int getPort() const;
	in_addr_t getServerIp() const;
	in_addr_t getClientIp() const;
	bool hasDisconnected() const ;
	bool getResponseIsReady() const;
	bool getRequestIsReady() const;
	bool getRequestParsed() const;
	bool didTimeout() const;
	HttpRequest& getRequest();
	HttpResponse& getResponse();

	/* Setters */
	void setHasDisconnected(bool status);
	void setResponse(HttpResponse res);

	/* Data Handling */
	void parseHttpHeader(string& buffer);
	void parseHttpBody(const string& buffer);
	void reset();

private:
	int m_id;
	int m_socketFd;
	bool m_hasDisconnected;
	bool m_responseIsReady;
	bool m_requestIsReady;
	bool m_requestParsed;
	bool m_recvChunk;
	time_t m_lastEventTime;
	Address m_serverAddr;
	Address m_clientAddr;
	HttpResponse m_response;
	HttpRequest m_request;

	string m_sessionCookie;
	string m_httpBuffer;
	string m_chunkedBuffer;
	int m_bytesRecved;
	int m_bytesExpected;
};
