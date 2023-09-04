#include "Server.hpp"

Server::Server(const ServerConfig config) :
    m_config(config)
{

    struct in_addr addr;
    addr.s_addr = m_config.address.host;
    log(INFO, "Server: %s on %s:%d", getName().c_str(), inet_ntoa(addr), m_config.address.port);
}

SocketAddress Server::getAddress()
{
    return m_config.address;
}

std::string Server::getName()
{
    return m_config.serverName;
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
