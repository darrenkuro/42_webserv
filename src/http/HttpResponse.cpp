#include "HttpResponse.hpp"

std::string getStatusText(int code)
{
	switch(code) {
		case 100:
			return "Continue";
		case 101:
			return "Switching Protocols";
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 203:
			return "Non-Authoritative Information";
		case 204:
			return "No Content";
		case 205:
			return "Reset Content";
		case 206:
			return "Partial Content";
		case 300:
			return "Multiple Choices";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 303:
			return "See Other";
		case 304:
			return "Not Modified";
		case 307:
			return "Temporary Redirect";
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 407:
			return "Proxy Authentication Required";
		case 408:
			return "Request Timeout";
		case 409:
			return "Conflict";
		case 410:
			return "Gone";
		case 411:
			return "Length Required";
		case 412:
			return "Precondition Failed";
		case 413:
			return "Payload Too Large";
		case 414:
			return "URI Too Long";
		case 415:
			return "Unsupported Media Type";
		case 416:
			return "Range Not Satisfiable";
		case 417:
			return "Expectation Failed";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		default:
			return "Unknown";
	}
}

std::string getMimeType(std::string ext)
{
	if (ext == ".aac") {
		return "audio/aac";
	}
	if (ext == ".css") {
		return "text/css";
	}
	if (ext == ".csv") {
		return "text/csv";
	}
	if (ext == ".gif") {
		return "image/git";
	}
	if (ext == ".htm" || ext == ".html") {
		return "text/html";
	}
	if (ext == ".ico") {
		return "image/vnd.microsoft.icon";
	}
	if (ext == ".jpeg" || ext == ".jpg") {
		return "image/jpeg";
	}
	if (ext == ".js" || ext == ".mjs") {
		return "text/javascript";
	}
	if (ext == ".json") {
		return "application/json";
	}
	if (ext == ".mp3") {
		return "audio/mpeg";
	}
	if (ext == ".mp4") {
		return "video/mp4";
	}
	if (ext == ".mpeg") {
		return "video/mpeg";
	}
	if (ext == ".png") {
		return "image/png";
	}
	if (ext == ".pdf") {
		return "application/pdf";
	}
	if (ext == ".php") {
		return "application/x-httpd-php";
	}
	if (ext == ".svg") {
		return "image/svg+xml";
	}
	if (ext == ".txt") {
		return "text/plain";
	}
	if (ext == ".wav") {
		return "audio/wav";
	}
	if (ext == ".webp") {
		return "image/webp";
	}
	return "text/plain";
}

std::string toString(HttpResponse res) {
	std::string str;
	str.append("HTTP/1.1 ");
	str.append(toString(res.statusCode) + " ");
	str.append(getStatusText(res.statusCode) + "\r\n");

	std::map<std::string, std::string>::iterator it;
	for (it = res.header.begin(); it != res.header.end(); it++) {
		str.append(it->first + ": ").append(it->second + "\r\n");
	}
	str.append("\r\n").append(res.body);
	return str;
}

std::string getDate(void)
{
	std::time_t t = std::time(NULL);
	std::tm* timePtr = std::gmtime(&t);

	char buffer[50];
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timePtr);
	return std::string(buffer);
}

HttpResponse createBasicResponse(int code, std::string path)
{
	HttpResponse res;

	res.statusCode = code;
	res.header["Date"] = getDate();
	res.header["Server"] = "Webserv42/1.0.0";

	// Handle POST response
	if (code == 204) {
		return res;
	}

	// Handle redirect response
	if (ConfigParser::isValidRedirectCode(code)) {
		res.header["Location"] = path;
		return res;
	}

	try {
		res.body = getFileContent(path);
		res.header["Content-Length"] = toString(res.body.length());
		res.header["Content-Type"] = getMimeType(getExtension(path));
		//res.header["Set-Cookie"] = "username=darren";
	}
	catch (...) {
		std::ifstream file(DEFAULT_404_PATH);
		res.body = file.is_open() ? getFileContent(DEFAULT_404_PATH) : "404 Not found";
		res.header["Content-Type"] = file.is_open() ? "text/html" : "text/plain";
		res.header["Content-Length"] = toString(res.body.length());
		file.close();
	}
	return res;
}
