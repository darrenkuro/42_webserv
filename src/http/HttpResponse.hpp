#pragma once

#include <string>
#include <map>

#define DEFAULT_400_PATH "./public/default_error/400.html"
#define DEFAULT_404_PATH "./public/default_error/404.html"

struct HttpResponse
{
	int statusCode;
	std::map<std::string, std::string> header;
	std::string body;
};

// Helper Functions
std::string getDate(void);
std::string getStatusText(int code);
std::string getMimeType(std::string path);
std::string toString(HttpResponse res);

// Main Function
HttpResponse createBasicResponse(int code, std::string path);
