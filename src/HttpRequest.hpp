#pragma once

#include <string>
#include <map>
#include <iostream>
#include <exception>

struct HttpRequest
{
	std::string method;
	std::string url;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
};

// Main Parse Function
HttpRequest parseHttpRequest(std::string content);
