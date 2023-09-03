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
    // if (request.method == "POST") {
    //     return handlePostRequest(request, route);
    // }
    // if (request.method == "DELETE") {
    //     return handleDeleteRequest(request, route);
    // }

    return createBasicResponse(501, m_config.errorPages[501], "text/html");
}

HttpResponse Server::handleGetRequest(HttpRequest request, LocationConfig route)
{
    std::string root = route.alias == "" ? m_config.root + route.uri : m_config.root + route.alias;
    std::string path = root + request.uri.substr(route.uri.length());

    std::cout << "path: " << path << std::endl;
    // First handle redirection?

    // UGH messy logic, clean up later
    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) == 0) {
        if (S_ISDIR(fileInfo.st_mode)) {
            for (std::vector<std::string>::iterator it = route.index.begin(); it != route.index.end(); it++) {
                std::string filePath = path + *it;
                std::ifstream file(filePath.c_str());
                if (file.good()) {
                    return createBasicResponse(200, filePath, "text/html");
                }
            }
            if (route.autoindex) {
                // build html for the directory file info
                return buildAutoindex(path);
            }
            else {
                return createBasicResponse(403, m_config.root + m_config.errorPages[403], "text/html");
            }
        }
        else if (S_ISREG(fileInfo.st_mode)) {
            // check file types
            return createBasicResponse(200, path, "text/html");
        }
        else {
            return createBasicResponse(403, m_config.root + m_config.errorPages[403], "text/html");
        }
    }
    else {
        return createBasicResponse(404, m_config.root + m_config.errorPages[404], "text/html");
    }
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
    return routeRequest(uri);

    // Didn't match anything, either function is wrong or the Server doesn't have '/' route
    throw std::runtime_error("Couldn't match uri " + uri);
}

// Move later
HttpResponse buildAutoindex(std::string path)
{
    std::string body("<!DOCTYPE html>");

    body += "<html><head><title>Directory Index</title></head>";
    body += "<body><h1>Index of " + path + "</h1>";

    body += "</body></html>";

    HttpResponse response;
    response.version = "HTTP/1.1";
    response.statusCode = 200;
    response.statusText = getStatusText(200);
    response.body = body;
    return response;
}
