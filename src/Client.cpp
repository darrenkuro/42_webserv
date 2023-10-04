#include "Client.hpp"
#include <ctime>		// time
#include <cstdlib>		// strtol

#define TIMEOUT_TIME	30

/* ============================================================================================== */
/*                                                                                                */
/*                                  Client Class Implementation                                   */
/*                                                                                                */
/* ============================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Constructors & Destructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
Client::Client(int fd, Address addr)
{
	static int clientID = 1;
	m_id = clientID;
	clientID++;

	m_socketFd = fd;
	m_address = addr;
	m_responseIsReady = false;
	m_hasDisconnected = false;

	m_lastEventTime = std::time(NULL);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Getters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int Client::getId() const { return m_id; }
int Client::getFd() const { return m_socketFd; }
int Client::getPort() const { return m_address.port; }
in_addr_t Client::getIp() const { return m_address.ip; }
bool Client::hasDisconnected() const { return m_hasDisconnected; }
bool Client::getResponseIsReady() const { return m_responseIsReady; }
bool Client::getRequestIsReady() const { return m_requestIsReady; }
bool Client::getRequestParsed() const { return m_requestParsed; }
bool Client::getRecvChunk() const { return m_recvChunk; }
bool Client::didTimeout() const { return std::time(NULL) - m_lastEventTime > TIMEOUT_TIME; }
HttpRequest& Client::getRequest() { return m_request; }
HttpResponse& Client::getResponse() { return m_responseIsReady = false, m_response; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void Client::setHasDisconnected(bool status) { m_hasDisconnected = status; }
void Client::setRecvChunk(bool recvChunk) { m_recvChunk = recvChunk; }

void Client::setRequest(HttpRequest req)
{
	m_request = req;
	m_requestParsed = true;
}

/* ---------------------------------------------------------------------------------------------- */
void Client::setResponse(HttpResponse res)
{
	m_response = res;
	m_lastEventTime = std::time(NULL);
	m_responseIsReady = true;
	m_requestParsed = false;
}

/* ---------------------------------------------------------------------------------------------- */
void Client::setBytesExpected(int bytes)
{
	m_bytesExpected = bytes;
	m_requestIsReady = false;
}

/* ---------------------------------------------------------------------------------------------- */
void Client::reset()
{
	m_bytesExpected = 0;
	m_bytesRecved = 0;
	m_requestIsReady = true;
	m_requestParsed = false;
}

/* ---------------------------------------------------------------------------------------------- */
void Client::appendData(string buffer)
{
	// if (m_recvChunk) {
	// 	string chunkSizeStr = buffer.substr(0, buffer.find("\r\n"));
	// 	buffer.erase(buffer.begin(), buffer.begin() + buffer.find("\r\n") + 2);
	// 	// check endptr?
	// 	int chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
	// 	if (chunkSize == 0) {
	// 		m_recvChunk = false;
	// 		m_requestIsReady = true;
	// 		return;
	// 	}
	// 	buffer.erase(chunkSize);
	// 	m_request.body.append(buffer);
	// }
	// else {
		m_request.body.append(buffer);
		m_bytesRecved += buffer.length();
		if (m_bytesRecved >= m_bytesExpected) {
			m_requestIsReady = true;
	 	}
	// }
}
