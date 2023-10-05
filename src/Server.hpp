#pragma once

#include "common.hpp"

class Server
{
public:
	Server(const ServerConfig config);

	// Getters
	Address getAddress() const;
	string getName() const;
	string getErrorPage(int code) const;

	// Logic
	HttpResponse handleRequest(HttpRequest req);
	HttpResponse handleGetRequest(HttpRequest req, LocationConfig route);
	HttpResponse handlePostRequest(HttpRequest req, LocationConfig route);
	HttpResponse handleDeleteRequest(HttpRequest req, LocationConfig route);

private:
	ServerConfig m_config;

	LocationConfig routeRequest(string uri);
	string getBoundry(HttpRequest req);
	int getMaxBodySize();
	bool bodySizeAllowed(int bytes);
};
