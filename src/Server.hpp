#pragma once

#include <string>
#include <algorithm>
#include <cstdlib>
#include "Webserver.hpp"
#include "utils.hpp"
#include <dirent.h>
#include <cstdio>
#include <sys/stat.h> // move to utils later?

struct ServerConfig;

class Server
{
public:
    Server(const ServerConfig config);

    SocketAddress getAddress();
    HttpResponse handleRequest(HttpRequest request);

    HttpResponse handleGetRequest(HttpRequest request, LocationConfig route);
    HttpResponse handlePostRequest(HttpRequest request, LocationConfig route);
    HttpResponse handleDeleteRequest(HttpRequest request, LocationConfig route);

private:
    ServerConfig m_config;

    LocationConfig routeRequest(std::string uri);
};

HttpResponse buildAutoindex(std::string path);
std::string fullPath(std::string root, std::string path);
