#include "HttpResponse.hpp"
#include "utils.hpp"

#include <iostream>
#include <fstream>

HttpResponse::HttpResponse() {
	initStatusMap();
}
HttpResponse::~HttpResponse() {}
HttpResponse::HttpResponse(const HttpResponse &rhs) { *this = rhs; }
HttpResponse &HttpResponse::operator=(const HttpResponse &rhs) {(void)rhs; return *this;}

HttpResponse::HttpResponse(const int status, const std::string page): m_statusCode(status) {
	m_version = "HTTP/1.1";
	try {
		m_body = getFileContent(page);
	} catch (std::exception &e) {
		throw std::runtime_error("[Error] Failed to load page " + page);
	}
}

std::string HttpResponse::toString() {
	std::string statusText = m_statusMap.find(m_statusCode) == m_statusMap.end() ? m_statusMap.find(m_statusCode)->second : "Unknown";
	std::string res = m_version + " " + intToString(m_statusCode) + " " + statusText + "\r\n";
	for (std::map<std::string, std::string>::iterator it = m_header.begin(); it != m_header.end(); it++) {
		res = res + it->first + ": " + it->second + "\r\n";
	}
	res = res + "\r\n" + m_body;
	return res;
}

size_t HttpResponse::length() {
	return toString().length();
}

/**
 * https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#information_responses
*/
void HttpResponse::initStatusMap()
{
	m_statusMap[100] = "Continue";
	m_statusMap[101] = "Switching Protocols";
	m_statusMap[102] = "Processing";
	m_statusMap[103] = "Early Hints";
	m_statusMap[200] = "OK";
	m_statusMap[201] = "Created";
	m_statusMap[202] = "Accepted";
	m_statusMap[203] = "Non-Authoritative Information";
	m_statusMap[204] = "No Content";
	m_statusMap[205] = "Reset Content";
	m_statusMap[206] = "Partial Content";
	m_statusMap[207] = "Multi-Status";
	m_statusMap[208] = "Already Reported";
	m_statusMap[226] = "IM Used";
	m_statusMap[300] = "Multiple Choices";
	m_statusMap[301] = "Moved Permanently";
	m_statusMap[302] = "Found";
	m_statusMap[303] = "See Other";
	m_statusMap[304] = "Not Modified";
	m_statusMap[307] = "Temporary Redirect";
	m_statusMap[308] = "Permanent Redirect";
	m_statusMap[400] = "Bad Request";
	m_statusMap[401] = "Unauthorized";
	m_statusMap[402] = "Payment Required";
	m_statusMap[403] = "Forbidden";
	m_statusMap[404] = "Not Found";
	m_statusMap[405] = "Method Not Allowed";
	m_statusMap[406] = "Not Acceptable";
	m_statusMap[407] = "Proxy Authentication Required";
	m_statusMap[408] = "Request Timeout";
	m_statusMap[409] = "Conflict";
	m_statusMap[410] = "Gone";
	m_statusMap[411] = "Length Required";
	m_statusMap[412] = "Precondition Failed";
	m_statusMap[413] = "Payload Too Large";
	m_statusMap[414] = "URI Too Long";
	m_statusMap[415] = "Unsupported Media Type";
	m_statusMap[416] = "Range Not Satisfiable";
	m_statusMap[417] = "Expectation Failed";
	m_statusMap[418] = "I'm a teapot";
	m_statusMap[421] = "Misdirected Request";
	m_statusMap[422] = "Unprocessable Content";
	m_statusMap[423] = "Locked";
	m_statusMap[424] = "Failed Dependency";
	m_statusMap[425] = "Too Early";
	m_statusMap[426] = "Upgrade Required";
	m_statusMap[428] = "Precondition Required";
	m_statusMap[429] = "Too Many Requests";
	m_statusMap[431] = "Request Header Fields Too Large";
	m_statusMap[451] = "Unavailable For Legal Reasons";
	m_statusMap[500] = "Internal Server Error";
	m_statusMap[501] = "Not Implemented";
	m_statusMap[502] = "Bad Gateway";
	m_statusMap[503] = "Service Unavailable";
	m_statusMap[504] = "Gateway Timeout";
	m_statusMap[505] = "HTTP Version Not Supported";
	m_statusMap[506] = "Variant Also Negotiates";
	m_statusMap[507] = "Insufficient Storage";
	m_statusMap[508] = "Loop Detected";
	m_statusMap[509] = "Not Extended";
	m_statusMap[511] = "Network Authentication Required";
}

/*

// assuming http request was validated, very simple version
HttpResponse::HttpResponse(const HttpRequest request)
{

}

std::string extractPath(std::string url)
{

}

void HttpResponse::handleGetRequest()
{
	m_body = getFileContent(request.url);
}
*/
