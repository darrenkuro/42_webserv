#pragma once

#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include "Webserver.hpp"
#include "utils.hpp"
#include "global.hpp"

struct ServerConfig;

typedef map<string, string> Cookies;

class Server
{
public:
	Server(const ServerConfig config);

	// Getters
	Address getAddress();
	string getName();
	string getErrorPage(int code);

	// Logic
	HttpResponse handleRequest(HttpRequest req);
	HttpResponse handleGetRequest(HttpRequest req, LocationConfig route);
	HttpResponse handlePostRequest(HttpRequest req, LocationConfig route);
	HttpResponse handleDeleteRequest(HttpRequest req, LocationConfig route);

	HttpResponse createBasicResponse(int code, string path);

private:
	ServerConfig m_config;
	map<string, Cookies> m_cookies;

	LocationConfig routeRequest(string uri);
	HttpResponse buildAutoindex(string path);
	string getBoundry(HttpRequest req);
	int getMaxBodySize();
	bool bodySizeAllowed(int bytes);
};
