#include "Client.hpp"
#include <ctime>		// time
#include <cstdlib>		// strtol
#include "http.hpp"		// parseHttpRequest
#define TIMEOUT_TIME	30

/* ============================================================================================== */
/*                                                                                                */
/*                                  Client Class Implementation                                   */
/*                                                                                                */
/* ============================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Constructors & Destructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
Client::Client(int fd, Address serverAddr, Address clientAddr)
{
	static int clientID = 1;
	m_id = clientID;
	clientID++;

	m_socketFd = fd;
	m_serverAddr = serverAddr;
	m_clientAddr = clientAddr;
	m_responseIsReady = false;
	m_hasDisconnected = false;
	m_bytesExpected = 0;
	m_bytesRecved = 0;
	m_requestIsReady = true;
	m_requestParsed = false;
	m_recvChunk = false;

	m_lastEventTime = std::time(NULL);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Getters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int Client::getId() const { return m_id; }
int Client::getFd() const { return m_socketFd; }
int Client::getPort() const { return m_serverAddr.port; }
in_addr_t Client::getServerIp() const { return m_serverAddr.ip; }
in_addr_t Client::getClientIp() const { return m_clientAddr.ip; }
bool Client::hasDisconnected() const { return m_hasDisconnected; }
bool Client::getResponseIsReady() const { return m_responseIsReady; }
bool Client::getRequestIsReady() const { return m_requestIsReady; }
bool Client::getRequestParsed() const { return m_requestParsed; }
bool Client::getRecvChunk() const { return m_recvChunk; }
bool Client::didTimeout() const { return std::time(NULL) - m_lastEventTime > TIMEOUT_TIME; }
string Client::getHeaderBuffer() const { return m_headerBuffer; }
HttpRequest& Client::getRequest() { return m_request; }
HttpResponse& Client::getResponse() { return m_responseIsReady = false, m_response; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void Client::setHasDisconnected(bool status) { m_hasDisconnected = status; }
void Client::setRecvChunk(bool recvChunk) {
	m_recvChunk = recvChunk;
	m_requestIsReady = false;
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
	m_headerBuffer.clear();
	m_chunkedBuffer.clear();
	m_recvChunk = false;
}

void Client::appendHeader(string buffer) { m_headerBuffer.append(buffer); }

/* ---------------------------------------------------------------------------------------------- */
void Client::appendBody(string buffer)
{
	if (!m_recvChunk) {
		m_request.body.append(buffer);
		m_bytesRecved += buffer.length();
		if (m_bytesRecved >= m_bytesExpected) {
			m_requestIsReady = true;
		}
	}
	else {
		m_chunkedBuffer.append(buffer);
		while (true) {
			size_t pos = m_chunkedBuffer.find("\r\n");
			if (pos == string::npos) break;

			string sizeStr = m_chunkedBuffer.substr(0, pos);
			int size = strtol(sizeStr.c_str(), NULL, 16);
			if (size == 0) {
				m_requestIsReady = true;
				m_chunkedBuffer.clear();
				break;
			}

			if (m_chunkedBuffer.size() < pos + 2 + size + 2) break;

			m_request.body.append(m_chunkedBuffer.substr(pos + 2, size));
			m_chunkedBuffer.erase(0, pos + 2 + size + 2);
		}
	}
}

void Client::parseHttpHeader()
{
	m_request = parseHttpRequest(m_headerBuffer);
	m_requestParsed = true;
}
