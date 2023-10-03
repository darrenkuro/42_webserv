#include <algorithm>
#include "Server.hpp"
#include "utils.hpp"

/* ============================================================================================== */
/*                                                                                                */
/*                                  Server Class Implementation                                   */
/*                                                                                                */
/* ============================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Constructors & Destructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
Server::Server(const ServerConfig config) : m_config(config)
{
	log(INFO, "Server: %s on %s:%d", getName().c_str(),
		toIPString(m_config.address.ip).c_str(), m_config.address.port);
}

Address Server::getAddress() { return m_config.address; }

string Server::getName() { return m_config.serverName; }

string Server::getErrorPage(int code) {
	return fullPath(m_config.root, m_config.errorPages[code]);
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::handleRequest(HttpRequest req)
{
	LocationConfig route = routeRequest(req.uri);

	// Check http version
	if (req.version != HTTP_VERSION) {
		return createBasicResponse(505, getErrorPage(505));
	}

	// Check client max body size
	map<string, string>::iterator it;
	it = req.header.find("Content-Length");
	if (it != req.header.end() && !bodySizeAllowed(toInt(it->second))) {
		return createBasicResponse(413, getErrorPage(413));
	}

	// Check if PUT without Content-Length
	if (req.method == "PUT" && it == req.header.end()) {
		return createBasicResponse(411, getErrorPage(411));
	}

	// Check if method is allowed
	vector<string> methods = route.allowedMethods;
	if (std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
		return createBasicResponse(405, getErrorPage(405));
	}

	try {
		if (req.method == "GET") return handleGetRequest(req, route);
		if (req.method == "POST") return handlePostRequest(req, route);
		if (req.method == "DELETE") return handleDeleteRequest(req, route);
	}
	catch (...) {
		return createBasicResponse(500, getErrorPage(500));
	}

	return createBasicResponse(500, getErrorPage(500));
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::handleGetRequest(HttpRequest req, LocationConfig route)
{
	string root = route.alias == ""
						? fullPath(m_config.root, route.uri)
						: fullPath(ROOT, route.alias);
	string path = req.uri.substr(route.uri.length()) == ""
						? root
						: fullPath(root, req.uri.substr(route.uri.length()));

	// Handle redirection
	if (route.redirect.first) {
		return createBasicResponse(route.redirect.first, route.redirect.second);
	}

	Stat fileInfo;
	if (stat(path.c_str(), &fileInfo) != 0) {
		return createBasicResponse(404, getErrorPage(404));
	}
	if (S_ISREG(fileInfo.st_mode)) {
		return createBasicResponse(200, path);
	}
	if (S_ISDIR(fileInfo.st_mode)) {
		vector<string>::iterator it;
		for (it = route.index.begin(); it != route.index.end(); it++) {
			string filePath = fullPath(path, *it);
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

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::handlePostRequest(HttpRequest req, LocationConfig route)
{
	string root =
		route.alias == ""
			? fullPath(m_config.root, route.uri)
			: fullPath(ROOT, route.alias);

	log(DEBUG, "root: %s", root.c_str());

	try {
		string boundry = getBoundry(req);

		if (req.body.find(boundry) == string::npos
			|| req.body.find("filename=\"") == string::npos) {
			throw exception();
		}

		// Get file name
		string filename;
		size_t nameStart = req.body.find("filename=\"") + 10;
		size_t nameEnd = req.body.find("\"", nameStart);
		filename = req.body.substr(nameStart, nameEnd - nameStart);

		// Trim to file content only
		req.body = req.body.substr(req.body.find("\r\n\r\n") + 4);
		if (req.body.find("\r\n" + boundry + "--") != string::npos) {
			req.body = req.body.substr(0, req.body.find("\r\n" + boundry + "--"));
		}

		// Save the file content to a file
		std::ofstream outputFile(fullPath(root, filename).c_str());
		if (!outputFile.is_open()) {
			throw exception();
		}
		outputFile << req.body;
		outputFile.close();

	}
	catch (...) {
		return createBasicResponse(400, getErrorPage(400));
	}

	return createBasicResponse(204, "");
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::handleDeleteRequest(HttpRequest req, LocationConfig route)
{
	string root =
		route.alias == ""
			? fullPath(m_config.root, route.uri)
			: fullPath(ROOT, route.alias);
	string path =
		req.uri.substr(route.uri.length()) == ""
			? root
			: fullPath(root, req.uri.substr(route.uri.length()));

	if (std::remove(path.c_str()) == 0) {
		return createBasicResponse(204, "");
	}

	return createBasicResponse(403, getErrorPage(403));
}

/* ---------------------------------------------------------------------------------------------- */
LocationConfig Server::routeRequest(string uri)
{
	vector<LocationConfig>::iterator it;

	// All Server config will have default '/' location
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

/* ---------------------------------------------------------------------------------------------- */
string Server::getBoundry(HttpRequest req)
{
	map<string, string>::iterator it;
	it = req.header.find("Content-Type");
	if (it == req.header.end()) {
		throw exception();
	}

	size_t pos = it->second.find("boundary=");
	if (pos == string::npos) {
		throw exception();
	}
	return "--" + it->second.substr(pos + 9);
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::buildAutoindex(string path)
{
	string body("<!DOCTYPE html>");

	body.append("<html><head><title>Directory Index</title>");
	body.append("<link rel=\"stylesheet\" type=\"text/css\" ");
	body.append("href=\"/style/autoindex.css\"></head>");
	body.append("<body><div class=\"container\"><h1 class=\"heading\">");
	body.append("Directory Autoindex</h1><ul class=\"list\">");

	DIR* dir;
	dirent* entry;
	if ((dir = opendir(path.c_str())) == NULL) {
		throw runtime_error("opendir failed");
	}

	while ((entry = readdir(dir)) != NULL) {
		string name(entry->d_name);
		if (name == "." || name == "..") {
			continue;
		}

		struct stat fileInfo;
		string filePath = fullPath(path, name);
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

/* ---------------------------------------------------------------------------------------------- */
int Server::getMaxBodySize() { return m_config.clientMaxBodySize; }

bool Server::bodySizeAllowed(int bytes)
{
	// If clientMaxBodySize is not set (-1) or larger
	return getMaxBodySize() == -1 || getMaxBodySize() >= bytes;
}

// StringMap generateCgiEnv(HttpRequest& req, const Client& client)
// {
// 	StringMap metaVars;
// 	metaVars["AUTH_TYPE"] = "Basic";
// 	metaVars["CONTENT_LENGTH"] = "";
// 	metaVars["CONTENT_TYPE"] = "";
// 	metaVars["GATEWAY_INTERFACE"] = "CGI/1.1";
// 	metaVars["PATH_INO"] = "";
// 	metaVars["PATH_TRANSLATED"] = "";
// 	metaVars["QUERY_STRING"] = "";
// 	metaVars["REMOTE_ADDR"] = "";
// 	metaVars["REMOTE_HOST"] = "";
// 	metaVars["REMOTE_IDENT"] = "";
// 	metaVars["REMOTE_USER"] = "";
// 	metaVars["REQUEST_METHOD"] = req.method;
// 	metaVars["SCRIPT_NAME"] = "";
// 	metaVars["SERVER_NAME"] = req.header["Host"]; // ?
// 	metaVars["SERVER_PORT"] = toString(client.getPort());
// 	metaVars["SERVER_PROTOCOL"] = HTTP_VERSION;
// 	metaVars["SERVER_SOFTWARE"] = SERVER_NAME;
// 	return metaVars;
// }
