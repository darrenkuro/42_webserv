#pragma once

#include <string>
#include <map>

struct HttpRequest
{
	std::string method;
	std::string uri;
	std::string version;
	std::string body;
	std::map<std::string, std::string> header;
};

// Main Function
HttpRequest parseHttpRequest(std::string content);

