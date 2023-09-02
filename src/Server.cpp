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

HttpResponse Server::handleRequest(HttpRequest request)
{
    LocationConfig route = routeRequest(request.uri);

    // Check if method is allowed
    if (std::find(route.allowedMethods.begin(), route.allowedMethods.end(), request.method) == route.allowedMethods.end()) {
        return createBasicResponse(405, m_config.errorPages[405], "text/html");
    }

    if (request.method == "GET") {
        return handleGetRequest(request, route);
    }
    if (request.method == "POST") {
        return handlePostRequest(request, route);
    }
    if (request.method == "DELETE") {
        return handleDeleteRequest(request, route);
    }

    return createBasicResponse(501, m_config.errorPages[501], "text/html");
}

HttpResponse Server::handleGetRequest(HttpRequest request, LocationConfig route)
{
    std::string root = route.alias == "" ? m_config.root + route.uri : m_config.root + route.alias;
    std::string path = request.uri.compare(0, route.uri.length(), route.uri) == 0
                ? request.uri.substr(route.uri.length() + 1) // remove trailing /
                : route.uri;

    // If request is for a directory

    // Check file types?

}

// HttpResponse Server::handlePostRequest(HttpRequest request, LocationConfig route)
// {

// }

// HttpResponse Server::handleDeleteRequest(HttpRequest request, LocationConfig route)
// {

// }

LocationConfig Server::routeRequest(std::string uri)
{
    // All Server config should have default '/' location
    for (std::vector<LocationConfig>::iterator it = m_config.locations.begin(); it != m_config.locations.end(); it++) {
        if (it->uri == uri) {
            return *it;
        }
    }

    // Recursively match the less complete uri
    size_t endPos = uri.find_last_of('/');
	uri = endPos == 0 ? "/" : uri.substr(0, uri.find_last_of('/'));
    routeRequest(uri);

    // Didn't match anything, either function is wrong or the Server doesn't have '/' route
    throw std::runtime_error("Couldn't match uri " + uri);
}
