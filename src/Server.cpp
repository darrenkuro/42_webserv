#include "Server.hpp"

#include <algorithm>	// find
#include <sys/stat.h>	// struct stat
#include <cstdio>		// remove

#include "Logger.hpp"
#include "http.hpp"		// createHttpResponse
#include "utils.hpp"	// fullPath, toIpString, toInt, getBoundary

/* ============================================================================================== */
/*                                                                                                */
/*                                  Server Class Implementation                                   */
/*                                                                                                */
/* ============================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Constructors & Destructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
Server::Server(const ServerConfig config) : m_config(config)
{
	LOG_INFO
		<< "Server constructor: " << getName() << " on " << toIpString(m_config.address.ip)
		<< ":" << m_config.address.port;
}

Address Server::getAddress() const { return m_config.address; }

string Server::getName() const { return m_config.serverName; }

string Server::getErrorPage(int code) const {
	map<int, string>::const_iterator it = m_config.errorPages.find(code);
	return it == m_config.errorPages.end() ? "" : fullPath(ROOT, it->second);
}

int Server::getMaxBodySize() const { return m_config.clientMaxBodySize; }

/* ---------------------------------------------------------------------------------------------- */
bool Server::bodySizeAllowed(int bytes) const
{
	// If clientMaxBodySize is not set (-1) or larger
	return getMaxBodySize() == -1 || getMaxBodySize() >= bytes;
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::handleRequest(HttpRequest req)
{
	LocationConfig route = routeRequest(req.uri);

	// Check HTTP version
	if (req.version != HTTP_VERSION) {
		return createHttpResponse(505, getErrorPage(505));
	}

	// Check client max body size
	StringMap::iterator it = req.header.find("Content-Length");
	if (it != req.header.end() && !bodySizeAllowed(toInt(it->second))) {
		return createHttpResponse(413, getErrorPage(413));
	}
	if (!bodySizeAllowed(req.body.size())) {
		return createHttpResponse(413, getErrorPage(413));
	}

	// Check if POST without Content-Length
	if (req.method == "POST" && it == req.header.end()) {
		return createHttpResponse(411, getErrorPage(411));
	}

	// Check if method is allowed
	vector<string> methods = route.allowedMethods;
	if (std::find(methods.begin(), methods.end(), req.method) == methods.end()) {
		return createHttpResponse(405, getErrorPage(405));
	}

	try {
		if (req.method == "GET") return handleGetRequest(req, route);
		if (req.method == "POST") return handlePostRequest(req, route);
		if (req.method == "DELETE") return handleDeleteRequest(req, route);
	}
	catch (const exception& e) {
		LOG_DEBUG << "Server handleRequest exception: " << e.what();
	}

	return createHttpResponse(500, getErrorPage(500));
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::handleGetRequest(HttpRequest req, LocationConfig route)
{
	string root =
		route.alias == ""
			? fullPath(m_config.root, route.uri)
			: fullPath(ROOT, route.alias);
	string path =
		req.uri.substr(route.uri.length()) == ""
			? root
			: fullPath(root, req.uri.substr(route.uri.length()));

	// Handle redirection
	if (route.redirect.first) {
		return createHttpResponse(route.redirect.first, route.redirect.second);
	}

	Stat fileInfo;
	if (stat(path.c_str(), &fileInfo) != 0) {
		return createHttpResponse(404, getErrorPage(404));
	}
	if (S_ISREG(fileInfo.st_mode)) {
		return createHttpResponse(200, path);
	}
	if (S_ISDIR(fileInfo.st_mode)) {
		vector<string>::iterator it;
		for (it = route.index.begin(); it != route.index.end(); it++) {
			string filePath = fullPath(path, *it);
			ifstream file(filePath.c_str());
			if (file.good()) return createHttpResponse(200, filePath);
		}
		if (route.autoindex) {
			return createAutoindex(path, route.uri);
		}
		else {
			return createHttpResponse(403, getErrorPage(403));
		}
	}

	return createHttpResponse(500, getErrorPage(500));
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse Server::handlePostRequest(HttpRequest req, LocationConfig route)
{
	if (req.header["Content-Type"].find("multipart/form-data") != 0) {
		LOG_WARNING << "Post Content-Type isn't multipart/form-data";
		return createHttpResponse(204, "");
	}

	string root =
		route.alias == ""
			? fullPath(m_config.root, route.uri)
			: fullPath(ROOT, route.alias);

	try {
		string boundry = getBoundary(req);

		if (req.body.find(boundry) == string::npos
			|| req.body.find("filename=\"") == string::npos) {
			throw runtime_error("couldn't find boundry or filename");
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
		string path = fullPath(root, filename);
		ofstream outputFile(path.c_str());
		if (!outputFile.is_open()) {
			throw runtime_error("failed to create file " + path);
		}
		outputFile << req.body;
		outputFile.close();

	}
	catch (const exception& e) {
		LOG_DEBUG << "Server handlePostRequest exception: " << e.what();
		return createHttpResponse(400, getErrorPage(400));
	}

	return createHttpResponse(204, "");
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

	if (::remove(path.c_str()) == 0) {
		return createHttpResponse(204, "");
	}

	LOG_WARNING << "Delete request: " << path << " not found";
	return createHttpResponse(404, getErrorPage(404));
}

/* ---------------------------------------------------------------------------------------------- */
LocationConfig Server::routeRequest(string uri)
{
	vector<LocationConfig>::iterator it;

	// All Server config will have default '/' location
	for (it = m_config.locations.begin(); it != m_config.locations.end(); it++) {
		if (it->uri == uri) return *it;
	}

	if (uri == "/") {
		throw runtime_error("root location '/' not found");
	}

	// Recursively match the less complete uri
	size_t endPos = uri.find_last_of('/');
	uri = endPos == 0 ? "/" : uri.substr(0, uri.find_last_of('/'));
	return routeRequest(uri);
}
