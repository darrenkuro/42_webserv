#pragma once

#include <string>
#include <map>
#include <vector>
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

// class HttpResponse {
// public:
// 	HttpResponse();
// 	~HttpResponse();
// 	HttpResponse(const HttpRequest request);
// 	HttpResponse(const HttpResponse &rhs);
// 	HttpResponse &operator=(const HttpResponse &rhs);
// 	HttpResponse(const int status, const std::string page);

// 	std::string toString();
// 	size_t length();

// private:
// 	const HttpRequest m_request;
// 	std::string m_version;
// 	int m_statusCode;
// 	std::map<std::string, std::string> m_header;
// 	std::string m_body;

// 	void handleGetRequest(const HttpRequest &request);

// 	std::map<int, std::string> m_statusMap;
// 	void initStatusMap();
// };


/**
1. Identify method -> call appropiate function

Get control flow:
    - check if http version is supported by our server
    - url parsing. properly encoded etc, no invalid characetrs
    - is url valid? Does that path exist for our webpage
    - ?maybe do query parameters

    - validate any headers that are accepted by our server (relevant to our server functionality)

    info: validation can maybe be done by the http parser. shoudlnt be in http request builder i think

    even if something failed we need to call some build function. we can build error http response




Lets try serving this

GET /index.html HTTP/1.1
Host: example.com


HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 53

<!DOCTYPE html>
<html>
<head>
    <title>Minimal Webpage</title>
</head>
<body>
    <h1>Hello, World!</h1>
</body>
</html>
*/
