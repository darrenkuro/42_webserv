#pragma once

#include "common.hpp"

class Server
{
public:
	Server(const ServerConfig config);

	/* Getters */
	Address getAddress() const;
	string getName() const;
	string getErrorPage(int code) const;
	int getMaxBodySize() const;
	bool bodySizeAllowed(int bytes) const;

	/* Logic */
	HttpResponse handleRequest(HttpRequest req);
	HttpResponse handleGetRequest(HttpRequest req, LocationConfig route);
	HttpResponse handlePostRequest(HttpRequest req, LocationConfig route);
	HttpResponse handleDeleteRequest(HttpRequest req, LocationConfig route);

private:
	ServerConfig m_config;

	LocationConfig routeRequest(string uri);
};
