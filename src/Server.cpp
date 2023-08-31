#include "Server.hpp"

Server::Server(const ServerConfig config) :
    m_address(config.address),
    m_serverName(config.serverName)
{
    log(INFO, "Server created: name: [%s] port: [%d]", m_serverName.c_str(), m_address.port);
}

SocketAddress Server::getAddress()
{
    return m_address;
}
