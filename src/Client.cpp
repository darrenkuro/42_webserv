#include "Client.hpp"

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

	gettimeofday(&m_lastEventTime, NULL);
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
bool Client::didTimeout() const { return getTime().tv_sec - m_lastEventTime.tv_sec > TIMEOUT_TIME; }
HttpRequest& Client::getRequest() { return m_request; }
HttpResponse& Client::getResponse() { return m_responseIsReady = false, m_response; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void Client::setHasDisconnected(bool status) { m_hasDisconnected = status; }

void Client::setRequest(HttpRequest req)
{
	m_request = req;
	m_requestParsed = true;
}

/* ---------------------------------------------------------------------------------------------- */
void Client::setResponse(HttpResponse res)
{
	m_response = res;
	gettimeofday(&m_lastEventTime, NULL);
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
	//m_request = appendBody(m_request, buffer);
	m_request.body.append(buffer);
	m_bytesRecved += buffer.length();
	if (m_bytesRecved >= m_bytesExpected) {
		m_requestIsReady = true;
	}
}
