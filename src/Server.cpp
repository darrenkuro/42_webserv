#include "Server.hpp"

Server::Server(const ServerConfig config) : m_config(config)
{
	log(INFO, "Server: %s on %s:%d", getName().c_str(),
		toIPString(m_config.address.host).c_str(), m_config.address.port);
}

SocketAddress Server::getAddress() { return m_config.address; }

std::string Server::getName() { return m_config.serverName; }

std::string Server::getErrorPage(int code) {
	return fullPath(m_config.root, m_config.errorPages[code]);
}

int Server::getMaxBodySize() { return m_config.clientMaxBodySize; }

HttpResponse Server::handleRequest(HttpRequest req)
{
	LocationConfig route = routeRequest(req.uri);

	// Check http version
	if (req.version != "HTTP/1.1") {
		return createBasicResponse(505, getErrorPage(505));
	}

	// Check client max body size
	std::map<std::string, std::string>::iterator it;
	it = req.headers.find("Content-Length");
	if (it != req.headers.end() && !bodySizeAllowed(toInt(it->second))) {
		return createBasicResponse(413, getErrorPage(413));
	}

	// Check if PUT without Content-Length
	if (req.method == "PUT" && it == req.headers.end()) {
		return createBasicResponse(411, getErrorPage(411));
	}

	// Check if method is allowed
	std::vector<std::string> methods = route.allowedMethods;
	if (std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
		return createBasicResponse(405, getErrorPage(405));
	}

	try {
		if (req.method == "GET") {
			return handleGetRequest(req, route);
		}
		if (req.method == "POST") {
			return handlePostRequest(req, route);
		}
		if (req.method == "DELETE") {
			return handleDeleteRequest(req, route);
		}
	}
	catch (...) {
		return createBasicResponse(500, getErrorPage(500));
	}

	return createBasicResponse(500, getErrorPage(500));
}

HttpResponse Server::handleGetRequest(HttpRequest req, LocationConfig route)
{
	std::string root = route.alias == ""
						? fullPath(m_config.root, route.uri)
						: fullPath(m_config.root, route.alias);
	std::string path = fullPath(root, req.uri.substr(route.uri.length()));

	log(DEBUG, "path: %s", path.c_str());
	// First handle redirection?

	struct stat fileInfo;
	if (stat(path.c_str(), &fileInfo) != 0) {
		return createBasicResponse(404, getErrorPage(404));
	}
	if (S_ISREG(fileInfo.st_mode)) {
		return createBasicResponse(200, path);
	}
	if (S_ISDIR(fileInfo.st_mode)) {
		std::vector<std::string>::iterator it;
		for (it = route.index.begin(); it != route.index.end(); it++) {
			std::string filePath = fullPath(path, *it);
			std::ifstream file(filePath.c_str());
			if (file.good()) {
				return createBasicResponse(200, filePath);
			}
		}
		if (route.autoindex) {
			return buildAutoindex(path);
		}
		else {
			return createBasicResponse(403, getErrorPage(403));
		}
	}

	return createBasicResponse(500, getErrorPage(500));
}

HttpResponse Server::handlePostRequest(HttpRequest req, LocationConfig route)
{
	std::string root = route.alias == ""
					? fullPath(m_config.root, route.uri)
					: fullPath(m_config.root, route.alias);

	try {
		std::string boundry = getBoundry(req);

		if (req.body.find(boundry) == std::string::npos
			|| req.body.find("filename=\"") == std::string::npos) {
			throw std::exception();
		}

		// Get file name
		std::string filename;
		size_t nameStart = req.body.find("filename=\"") + 10;
		size_t nameEnd = req.body.find("\"", nameStart);
		filename = req.body.substr(nameStart, nameEnd - nameStart);

		// Trim to file content only
		req.body = req.body.substr(req.body.find("\r\n\r\n") + 4);
		if (req.body.find("\r\n" + boundry + "--") != std::string::npos) {
			req.body = req.body.substr(0, req.body.find("\r\n" + boundry + "--"));
		}

		// Save the file content to a file
		std::ofstream outputFile(fullPath(root, filename).c_str());
		if (!outputFile.is_open()) {
			throw std::exception();
		}
		outputFile << req.body;
		outputFile.close();

	}
	catch (...) {
		return createBasicResponse(400, getErrorPage(400));
	}

	return createBasicResponse(204, "");
}

HttpResponse Server::handleDeleteRequest(HttpRequest req, LocationConfig route)
{
	std::string root = route.alias == ""
						? fullPath(m_config.root, route.uri)
						: fullPath(m_config.root, route.alias);
	std::string path = fullPath(root, req.uri.substr(route.uri.length()));

	if (std::remove(path.c_str()) == 0) {
		return createBasicResponse(204, "");
	}

	return createBasicResponse(403, getErrorPage(403));
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
}

std::string Server::getBoundry(HttpRequest req)
{
	std::map<std::string, std::string>::iterator it;
	it = req.headers.find("Content-Type");
	if (it == req.headers.end()) {
		throw std::exception();
	}

	size_t pos = it->second.find("boundary=");
	if (pos == std::string::npos) {
		throw std::exception();
	}
	return "--" + it->second.substr(pos + 9);
}

HttpResponse Server::buildAutoindex(std::string path)
{
	std::string body("<!DOCTYPE html>");

	body.append("<html><head><title>Directory Index</title>");
	body.append("<link rel=\"stylesheet\" type=\"text/css\" ");
	body.append("href=\"/style/autoindex.css\"></head>");
	body.append("<body><div class=\"container\"><h1 class=\"heading\">");
	body.append("Directory Autoindex</h1><ul class=\"list\">");

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
		// add a href?
		body.append("<li class=\"list-item\"><div class=\"name\">");
		body.append(name).append("</div></li>");
	}
	body.append("</ul></div></body></html>");

	HttpResponse response;
	response.statusCode = 200;
	response.body = body;
	return response;
}

bool Server::bodySizeAllowed(int bytes)
{
	// If clientMaxBodySize is not set (-1) or larger
	return getMaxBodySize() == -1 || getMaxBodySize() >= bytes;
}
