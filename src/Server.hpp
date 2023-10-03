#pragma once

#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include "Webserver.hpp"
#include "utils.hpp"
#include "infrastructure.hpp"

struct ServerConfig;

typedef map<string, string> Cookies;

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
	map<string, Cookies> m_cookies;

	LocationConfig routeRequest(string uri);
	HttpResponse buildAutoindex(string path);
	string getBoundry(HttpRequest req);
	int getMaxBodySize();
	bool bodySizeAllowed(int bytes);
};
