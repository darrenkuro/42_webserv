#include "HttpRequest.hpp"

/* --------------------------------------------------------------------------------------------- *
 * Parse a field for HTTP request, with a given separator, consume the content.
 * --------------------------------------------------------------------------------------------- */
void parsePart(const std::string& sep, std::string& field, std::string& content)
{
	size_t pos = content.find(sep);
	if (pos == std::string::npos) {
		throw std::exception();
	}

	field = content.substr(0, pos);
	content.erase(content.begin(), content.begin() + pos + sep.length());

	// Check if there's more than one separator (invalid request)
	if (content.find(sep) == 0) {
		throw std::exception();
	}
}

/* --------------------------------------------------------------------------------------------- *
 * Parse a header field for HTTP request, consume the content.
 * --------------------------------------------------------------------------------------------- */
void parseHeader(std::map<std::string, std::string>& header, std::string& content)
{
	size_t colonPos = content.find(":");
	if (colonPos == std::string::npos) {
		throw std::exception();
	}

	std::string key = content.substr(0, colonPos);
	content.erase(content.begin(), content.begin() + colonPos + 1);

	// Handle leading white spaces before value
	size_t valuePos = content.find_first_not_of(" \t");
	if (valuePos == std::string::npos) {
		throw std::exception();
	}
	content.erase(content.begin(), content.begin() + valuePos);

	size_t crlfPos = content.find("\r\n");
	if (crlfPos == std::string::npos) {
		throw std::exception();
	}
	std::string value = content.substr(0, crlfPos);
	content.erase(content.begin(), content.begin() + crlfPos + 2);

	header[key] = value;
}

/* --------------------------------------------------------------------------------------------- */
HttpRequest parseHttpRequest(std::string& content)
{
	HttpRequest req;

	parsePart(" ", req.method, content);
	parsePart(" ", req.uri, content);
	parsePart("\r\n", req.version, content);
	while (content.find("\r\n") != 0) {
		parseHeader(req.header, content);
	}

	// Remove the blank line after the headers
	content.erase(content.begin(), content.begin() + 2);

	// GET & DELETE usually contains no relavant body
	return req;
}
