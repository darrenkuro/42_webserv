#include "HttpResponse.hpp"

std::pair<int, std::string> statusTextMap[] = {
	std::make_pair(100, "Continue"),
	std::make_pair(101, "Switching Protocols"),
	std::make_pair(200, "OK"),
	std::make_pair(201, "Created"),
	std::make_pair(202, "Accepted"),
	std::make_pair(203, "Non-Authoritative Information"),
	std::make_pair(204, "No Content"),
	std::make_pair(205, "Reset Content"),
	std::make_pair(206, "Partial Content"),
	std::make_pair(300, "Multiple Choices"),
	std::make_pair(301, "Moved Permanently"),
	std::make_pair(302, "Found"),
	std::make_pair(303, "See Other"),
	std::make_pair(304, "Not Modified"),
	std::make_pair(307, "Temporary Redirect"),
	std::make_pair(400, "Bad Request"),
	std::make_pair(401, "Unauthorized"),
	std::make_pair(403, "Forbidden"),
	std::make_pair(404, "Not Found"),
	std::make_pair(405, "Method Not Allowed"),
	std::make_pair(406, "Not Acceptable"),
	std::make_pair(407, "Proxy Authentication Required"),
	std::make_pair(408, "Request Timeout"),
	std::make_pair(409, "Conflict"),
	std::make_pair(410, "Gone"),
	std::make_pair(411, "Length Required"),
	std::make_pair(412, "Precondition Failed"),
	std::make_pair(413, "Payload Too Large"),
	std::make_pair(414, "URI Too Long"),
	std::make_pair(415, "Unsupported Media Type"),
	std::make_pair(416, "Range Not Satisfiable"),
	std::make_pair(417, "Expectation Failed"),
	std::make_pair(500, "Internal Server Error"),
	std::make_pair(501, "Not Implemented"),
	std::make_pair(502, "Bad Gateway"),
	std::make_pair(503, "Service Unavailable"),
	std::make_pair(504, "Gateway Timeout"),
	std::make_pair(505, "HTTP Version Not Supported")
};

HttpResponse createBasicResponse(int code, std::string path, std::string type)
{
	HttpResponse response;
	std::string content;
	try {
		content = getFileContent(path);
	}
	catch (std::exception &e) {
		content = getStatusText(code);
	}
	response.version = "HTTP/1.1";
	response.statusCode = code;
	response.statusText = getStatusText(code);
	// date &
	response.header.insert(std::make_pair("Content-Length",
		toString(content.length())));
	response.header.insert(std::make_pair("Content-Type", type));
	response.body = content;

	return response;
}

std::string getStatusText(int code)
{
	// (void)code;
	for (int i = 0; i < STATUS_MAP_SIZE; i++) {
		if (statusTextMap[i].first == code) {
			return statusTextMap[i].second;
		}
	}
	return "No status text provided";
}

std::string toString(HttpResponse response) {
	std::string str(response.version + " " + toString(response.statusCode) + " " + response.statusText + "\r\n");
	for (std::map<std::string, std::string>::iterator it = response.header.begin(); it != response.header.end(); it++) {
		str = str + it->first + ": " + it->second + "\r\n";
	}
	str += "\r\n";
	str = str + response.body;
	return str;
}
