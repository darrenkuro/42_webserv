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

private:
    SocketAddress m_address;
    std::string m_serverName;
};
