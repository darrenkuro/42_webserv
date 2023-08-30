#include "Server.hpp"

Server::Server(const ServerConfig config) :
    m_port(config.port),
    m_serverName(config.serverName)
{
    log(INFO, "Server created: name: [%s] port: [%d]", m_serverName.c_str(), m_port);
}

int Server::getPort()
{
    return m_port;
}
