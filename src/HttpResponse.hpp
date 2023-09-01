#pragma once

#include <string>
#include <map>
#include <vector>
#include "utils.hpp"

#define DEFAULT_400_PATH    "./public/default_error/400.html"
#define STATUS_MAP_SIZE 60

struct HttpResponse
{
	std::string version;
	int statusCode;
	std::string statusText;
	std::map<std::string, std::string> header;
	std::string body;
};

// std::pair<int, std::string> statusTextMap[61] = {
//     std::make_pair(100, "Continue"),
// 	std::make_pair(101, "Switching Protocols"),
// 	std::make_pair(102, "Processing"),
// 	std::make_pair(103, "Early Hints"),
// 	std::make_pair(200, "OK"),
// 	std::make_pair(201, "Created"),
// 	std::make_pair(202, "Accepted"),
// 	std::make_pair(203, "Non-Authoritative Information"),
// 	std::make_pair(204, "No Content"),
// 	std::make_pair(205, "Reset Content"),
// 	std::make_pair(206, "Partial Content"),
// 	std::make_pair(207, "Multi-Status"),
// 	std::make_pair(208, "Already Reported"),
// 	std::make_pair(226, "IM Used"),
// 	std::make_pair(300, "Multiple Choices"),
// 	std::make_pair(301, "Moved Permanently"),
// 	std::make_pair(302, "Found"),
// 	std::make_pair(303, "See Other"),
// 	std::make_pair(304, "Not Modified"),
// 	std::make_pair(307, "Temporary Redirect"),
// 	std::make_pair(308, "Permanent Redirect"),
// 	std::make_pair(400, "Bad Request"),
// 	std::make_pair(401, "Unauthorized"),
// 	std::make_pair(402, "Payment Required"),
// 	std::make_pair(403, "Forbidden"),
// 	std::make_pair(404, "Not Found"),
// 	std::make_pair(405, "Method Not Allowed"),
// 	std::make_pair(406, "Not Acceptable"),
// 	std::make_pair(407, "Proxy Authentication Required"),
// 	std::make_pair(408, "Request Timeout"),
// 	std::make_pair(409, "Conflict"),
// 	std::make_pair(410, "Gone"),
// 	std::make_pair(411, "Length Required"),
// 	std::make_pair(412, "Precondition Failed"),
// 	std::make_pair(413, "Payload Too Large"),
// 	std::make_pair(414, "URI Too Long"),
// 	std::make_pair(415, "Unsupported Media Type"),
// 	std::make_pair(416, "Range Not Satisfiable"),
// 	std::make_pair(417, "Expectation Failed"),
// 	std::make_pair(418, "I'm a teapot"),
// 	std::make_pair(421, "Misdirected Request"),
// 	std::make_pair(422, "Unprocessable Content"),
// 	std::make_pair(423, "Locked"),
// 	std::make_pair(424, "Failed Dependency"),
// 	std::make_pair(425, "Too Early"),
// 	std::make_pair(426, "Upgrade Required"),
// 	std::make_pair(428, "Precondition Required"),
// 	std::make_pair(429, "Too Many Requests"),
// 	std::make_pair(431, "Request Header Fields Too Large"),
// 	std::make_pair(451, "Unavailable For Legal Reasons"),
// 	std::make_pair(500, "Internal Server Error"),
// 	std::make_pair(501, "Not Implemented"),
// 	std::make_pair(502, "Bad Gateway"),
// 	std::make_pair(503, "Service Unavailable"),
// 	std::make_pair(504, "Gateway Timeout"),
// 	std::make_pair(505, "HTTP Version Not Supported"),
// 	std::make_pair(506, "Variant Also Negotiates"),
// 	std::make_pair(507, "Insufficient Storage"),
// 	std::make_pair(508, "Loop Detected"),
// 	std::make_pair(509, "Not Extended"),
// 	std::make_pair(511, "Network Authentication Required")
// };

std::string getStatusText(int code);
HttpResponse createBasicResponse(int code);
std::string toString(HttpResponse response);

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
