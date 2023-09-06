#pragma once

#include <string>
#include <cstdlib>
#include <cstdio>       // std::remove
#include <dirent.h>
#include <sys/stat.h>
#include "Webserver.hpp"
#include "utils.hpp"

struct ServerConfig;

class Server
{
public:
    Server(const ServerConfig config);

    SocketAddress getAddress();
    std::string getName();
    std::string getErrorPage(int code);

    HttpResponse handleRequest(HttpRequest req);
    HttpResponse handleGetRequest(HttpRequest req, LocationConfig route);
    HttpResponse handlePostRequest(HttpRequest req, LocationConfig route);
    HttpResponse handleDeleteRequest(HttpRequest req, LocationConfig route);

private:
    ServerConfig m_config;

    LocationConfig routeRequest(std::string uri);
    HttpResponse buildAutoindex(std::string path);
};
