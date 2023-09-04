#include "Server.hpp"

Server::Server(const ServerConfig config) :
    m_config(config)
{

    struct in_addr addr;
    addr.s_addr = m_config.address.host;
    log(INFO, "Server: %s on %s:%d", getName().c_str(), inet_ntoa(addr), m_config.address.port);
}

SocketAddress Server::getAddress() { return m_config.address; }
std::string Server::getName() { return m_config.serverName; }

HttpResponse Server::handleRequest(HttpRequest req)
{
    LocationConfig route = routeRequest(req.uri);

    // Check if method is allowed
    std::vector<std::string> methods = route.allowedMethods;
    if (std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
        return createBasicResponse(405, m_config.errorPages[405]);
    }

    try {
        if (req.method == "GET") {
            return handleGetRequest(req, route);
        }
        // if (request.method == "POST") {
        //     return handlePostRequest(request, route);
        // }
        if (req.method == "DELETE") {
            return handleDeleteRequest(req, route);
        }
    }
    catch (...) {
        return createBasicResponse(500, m_config.errorPages[500]);
    }

    return createBasicResponse(501, m_config.errorPages[501]);
}

HttpResponse Server::handleGetRequest(HttpRequest req, LocationConfig route)
{
    std::string root = route.alias == "" ? m_config.root + route.uri : m_config.root + route.alias;
    //std::string path = root + req.uri.substr(route.uri.length());
    std::string path = fullPath(root, req.uri.substr(route.uri.length()));

    log(DEBUG, "path: %s", path.c_str());
    // First handle redirection?

    // UGH messy logic, clean up later
    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) == 0) {
        if (S_ISDIR(fileInfo.st_mode)) {
            for (std::vector<std::string>::iterator it = route.index.begin(); it != route.index.end(); it++) {
                std::string filePath = fullPath(path, *it);
                std::ifstream file(filePath.c_str());
                if (file.good())
                    return createBasicResponse(200, filePath);
            }
            if (route.autoindex) {
                // build html for the directory file info
                return buildAutoindex(path);
            }
            else {
                return createBasicResponse(403, m_config.root + m_config.errorPages[403]);
            }
        }
        else if (S_ISREG(fileInfo.st_mode)) {
            // check file types
            return createBasicResponse(200, path);
        }
        else {
            return createBasicResponse(403, m_config.root + m_config.errorPages[403]);
        }
    }
    else {
        return createBasicResponse(404, m_config.root + m_config.errorPages[404]);
    }
}

// HttpResponse Server::handlePostRequest(HttpRequest request, LocationConfig route)
// {

// }

HttpResponse Server::handleDeleteRequest(HttpRequest request, LocationConfig route)
{
    std::string root = route.alias == "" ? m_config.root + route.uri : m_config.root + route.alias;
    std::string path = fullPath(root, request.uri.substr(route.uri.length()));

    if (std::remove(path.c_str()) == 0) {
        return createBasicResponse(204, "");
    }

    return createBasicResponse(403, m_config.root + m_config.errorPages[403]);
}

LocationConfig Server::routeRequest(std::string uri)
{
    std::vector<LocationConfig>::iterator it;

    // All Server config should have default '/' location
    for (it = m_config.locations.begin(); it != m_config.locations.end(); it++) {
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
HttpResponse Server::buildAutoindex(std::string path)
{
    std::string body("<!DOCTYPE html>");

    body += "<html><head><title>Directory Index</title><style>" + getFileContent("./public/style/autoindex.css") + "</style></head>";
    body += "<body><div class=\"container\"><h1 class=\"heading\">Directory Autoindex</h1><ul class=\"list\">";

    DIR* dir;
    dirent* entry;
    if ((dir = opendir(path.c_str())) == NULL) {
        throw std::runtime_error("opendir failed");
    }
    while ((entry = readdir(dir)) != NULL) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") {
            continue;
        }

        struct stat fileInfo;
        std::string filePath = fullPath(path, name);
        stat(filePath.c_str(), &fileInfo);
        name = S_ISDIR(fileInfo.st_mode) ? name + "/" : name;
        // add <a href=\"" + name + "\">?
        body += "<li class=\"list-item\"><div class=\"name\">" + name + "</div></li>";
    }
    body += "</ul></div></body></html>";

    HttpResponse response;
    response.statusCode = 200;
    response.body = body;
    return response;
}
