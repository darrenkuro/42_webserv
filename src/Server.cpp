#include "Server.hpp"

Server::Server(const ServerConfig config) :
    m_config(config)
{
    log(INFO, "Server created: name: [%s] port: [%d]", m_config.serverName.c_str(), m_config.address.port);
}

SocketAddress Server::getAddress()
{
    return m_config.address;
}

// HttpResponse Server::handleGetRequest(HttpRequest request)
// {

// }

// HttpResponse Server::handlePostRequest(HttpRequest request)
// {

// }
// HttpResponse Server::handleDeleteRequest(HttpRequest request)
// {

// }
