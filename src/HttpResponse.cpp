#include "HttpResponse.hpp"
#include "utils.hpp"

#include <iostream>
#include <fstream>

HttpResponse createBasicResponse(int code)
{
	HttpResponse response;
	response.version = "HTTP/1.1";
	response.statusCode = code;
	response.statusText = getStatusText(code);
	response.header.insert(std::make_pair("Content-Length",
		toString(response.statusText.length())));
	response.header.insert(std::make_pair("Content-Type", "text/plain"));
	response.body = response.statusText;

	return response;
}

std::string getStatusText(int code)
{
	(void)code;
	for (int i = 0; i < STATUS_MAP_SIZE; i++) {
		// if (statusTextMap[i].first == code) {
		// 	return statusTextMap[i].second;
		// }
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
