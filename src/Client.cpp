#include "Client.hpp"

#include <ctime>		// time
#include <cstdlib>		// strtol

#include "http.hpp"		// parseHttpRequest
#include "utils.hpp"	// toString

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
bool Client::didTimeout() const { return std::time(NULL) - m_lastEventTime > TIMEOUT_TIME; }
HttpRequest& Client::getRequest() { return m_request; }
HttpResponse& Client::getResponse() { return m_responseIsReady = false, m_response; }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void Client::setHasDisconnected(bool status) { m_hasDisconnected = status; }

/* ---------------------------------------------------------------------------------------------- */
void Client::setResponse(HttpResponse res)
{
	m_response = res;
	if (!m_sessionCookie.empty()) m_response.header["Set-Cookie"] = m_sessionCookie;

	m_lastEventTime = std::time(NULL);
	m_responseIsReady = true;
	m_requestParsed = false;
}

/* ---------------------------------------------------------------------------------------------- */
void Client::reset()
{
	m_bytesExpected = 0;
	m_bytesRecved = 0;
	m_requestIsReady = true;
	m_requestParsed = false;
	m_httpBuffer.clear();
	m_chunkedBuffer.clear();
	m_recvChunk = false;
	m_sessionCookie = "";
}

/* ---------------------------------------------------------------------------------------------- */
void Client::parseHttpBody(const string& buffer)
{
	if (m_recvChunk) {
		// Handle chunked encoding
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
	else {
		// Handle multipart/form-data
		m_request.body.append(buffer);
		m_bytesRecved += buffer.length();
		if (m_bytesRecved >= m_bytesExpected) {
			m_requestIsReady = true;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */
void Client::parseHttpHeader(string& buffer)
{
	// Keep appending to buffer until it contains a "\r\n\r\n"
	m_httpBuffer.append(buffer);
	if (m_httpBuffer.find("\r\n\r\n") == string::npos) return;

	// Header buffer is ready, proceed to parse
	m_request = parseHttpRequest(m_httpBuffer);
	m_requestParsed = true;

	// Update buffer
	buffer = buffer.substr(buffer.find("\r\n\r\n") + 4);

	// Check Content-Length and set expecting bytes
	StringMap::iterator it = m_request.header.find("Content-Length");
	if (it != m_request.header.end()) {
		m_bytesExpected = toInt(it->second);
		m_requestIsReady = false;
	}

	// Check Transfer-Encoding and set chunked
	it = m_request.header.find("Transfer-Encoding");
	if (it != m_request.header.end() && it->second == "chunked") {
		m_recvChunk = true;
		m_requestIsReady = false;
	}

	// Check cookie for session management
	it = m_request.header.find("Cookie");
	if (it == m_request.header.end()) {
		m_sessionCookie = "session=" + toString(m_id);
	}
}
