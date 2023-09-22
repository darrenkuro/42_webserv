#pragma once

#include <string>
#include <map>
#include <exception>

struct HttpRequest
{
	std::string method;
	std::string uri;
	std::string version;
	std::string body;
	std::map<std::string, std::string> header;
};

// Main Parse Function
HttpRequest parseHttpRequest(std::string& content);

