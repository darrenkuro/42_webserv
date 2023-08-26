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
	std::map<std::string, std::string> header;
	std::string body;
};

struct HttpResponse
{
};

void parsePart(std::string sep, std::string &field, std::string &content);
void checkMethod(std::string method);
HttpRequest parseHttpRequest(std::string content);

