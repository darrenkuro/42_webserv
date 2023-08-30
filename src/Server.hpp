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

    int getPort();
private:
    int m_port;
    std::string m_serverName;
};