#include "HttpRequest.hpp"

//std::string validMethods[] = {"GET", "POST", "DELETE"};

// Method not included => 501
// void checkMethod(std::string method)
// {
// 	for (int i = 0; i < 3; i++) {
// 		if (method == validMethods[i]) {
// 			return;
// 		}
// 	}
// 	throw std::exception();
// }

void parsePart(std::string sep, std::string &field, std::string &content)
{
	if (content.find(sep) == std::string::npos)
		throw std::exception();
	field = content.substr(0, content.find(sep));
	content.erase(content.begin(), content.begin() + content.find(sep) + sep.length());
	if (content.find(sep) == 0)
		throw std::exception();
}

void parseHeader(std::map<std::string, std::string> &header, std::string &content)
{
	if (content.find(':') == std::string::npos)
		throw std::exception();
	std::string key = content.substr(0, content.find(':'));
	content.erase(content.begin(), content.begin() + content.find(':') + 1);

	// Handle leading white spaces before value
	size_t valuePos = content.find_first_not_of(" \t");
	if (valuePos == std::string::npos)
		throw std::exception();
	content.erase(content.begin(), content.begin() + valuePos);

	if (content.find("\r\n") == std::string::npos)
		throw std::exception();
	std::string value = content.substr(0, content.find("\r\n"));
	content.erase(content.begin(), content.begin() + content.find("\r\n") + 2);
	header[key] = value;
}

HttpRequest parseHttpRequest(std::string content)
{
	HttpRequest request;

	parsePart(" ", request.method, content);
	parsePart(" ", request.uri, content);
	parsePart("\r\n", request.version, content);
	while (content.find("\r\n") != 0) {
		parseHeader(request.headers, content);
	}

	// Remove the blank line after the headers
	content.erase(content.begin(), content.begin() + 2);
	request.body = content;
	return request;
}
