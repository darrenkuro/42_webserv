#pragma once

#include <string>
#include <map>
#include <exception>
#include <unistd.h>

struct HttpRequest
{
	std::string method;
	std::string uri;
	std::string version;
	std::string body;
	std::map<std::string, std::string> headers;
};

// Main Parse Function
HttpRequest parseHttpRequest(std::string& content);
HttpRequest& appendBody(HttpRequest& req, std::string body);
