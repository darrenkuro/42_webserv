#pragma once

#include <string>
#include <cstdlib>
#include "Webserver.hpp"
#include "utils.hpp"

struct ServerConfig;

class Server
{
public:
    Server(const ServerConfig config);

    SocketAddress getAddress();
    HttpResponse handleGetRequest(HttpRequest request);
    HttpResponse handlePostRequest(HttpRequest request);
    HttpResponse handleDeleteRequest(HttpRequest request);

private:
    ServerConfig m_config;
};
