#pragma once

#include <map>
#include <iostream>
#include <fstream>
#include <ctime>
#include "utils.hpp"

#define DEFAULT_400_PATH "./public/default_error/400.html"
#define DEFAULT_404_PATH "./public/default_error/404.html"

struct HttpResponse
{
	int statusCode;
	std::map<std::string, std::string> header;
	std::string body;
};

std::string getDate(void);
std::string getStatusText(int code);
std::string getMimeType(std::string path);
std::string toString(HttpResponse res);

HttpResponse createBasicResponse(int code, std::string path);
