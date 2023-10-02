#pragma once

#include <string>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include "Webserver.hpp"
#include "utils.hpp"

struct ServerConfig;

typedef std::map<std::string, std::string> Cookies;

class Server
{
public:
	Server(const ServerConfig config);

	// Getters
	Address getAddress();
	std::string getName();
	std::string getErrorPage(int code);

	// Logic
	HttpResponse handleRequest(HttpRequest req);
	HttpResponse handleGetRequest(HttpRequest req, LocationConfig route);
	HttpResponse handlePostRequest(HttpRequest req, LocationConfig route);
	HttpResponse handleDeleteRequest(HttpRequest req, LocationConfig route);

private:
	ServerConfig m_config;
	std::map<std::string, Cookies> m_cookies;

	LocationConfig routeRequest(std::string uri);
	HttpResponse buildAutoindex(std::string path);
	std::string getBoundry(HttpRequest req);
	int getMaxBodySize();
	bool bodySizeAllowed(int bytes);
};
