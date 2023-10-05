#include "http.hpp"
#include "ConfigParser.hpp"	// isValidRedirectCode
#include "utils.hpp"		// toString, getFileContent
#include <sys/stat.h>		// struct stat
#include <dirent.h>			// DIR, dirent, opendir, readdir, closedir
#include <ctime>			// time, gmtime, strftime
#include "log.hpp" // temp

/* ============================================================================================== */
/*                                                                                                */
/*                                 HTTP Functional Implementation                                 */
/*                                                                                                */
/* ============================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HTTP Request Parser ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
HttpRequest parseHttpRequest(string& content)
{
	HttpRequest req;

	parseRequestPart(" ", req.method, content);
	parseRequestPart(" ", req.uri, content);
	parseRequestPart("\r\n", req.version, content);
	while (content.find("\r\n") != 0) {
		parseRequestHeader(req.header, content);
	}

	// Remove the blank line after the headers
	content.erase(content.begin(), content.begin() + 2);

	// GET & DELETE usually contains no relavant body
	return req;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HTTP Request Helpers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void parseRequestPart(const string& sep, string& field, string& content)
{
	size_t pos = content.find(sep);
	if (pos == string::npos) {
		throw runtime_error("couldn't find '" + sep + "'");
	}

	field = content.substr(0, pos);
	content.erase(content.begin(), content.begin() + pos + sep.length());

	// Check if there's more than one separator (invalid request)
	if (content.find(sep) == 0) {
		throw runtime_error("extra '" + sep + "'");
	}
}

/* ---------------------------------------------------------------------------------------------- */
void parseRequestHeader(StringMap& header, string& content)
{
	size_t colonPos = content.find(":");
	if (colonPos == string::npos) {
		throw runtime_error("couldn't find ':'");
	}
	string key = content.substr(0, colonPos);
	content.erase(content.begin(), content.begin() + colonPos + 1);

	// Handle leading white spaces before value
	size_t valuePos = content.find_first_not_of(" \t");
	if (valuePos == string::npos)  {
		throw runtime_error("couldn't find ' '");
	}
	content.erase(content.begin(), content.begin() + valuePos);

	size_t crlfPos = content.find("\r\n");
	if (crlfPos == string::npos) {
		throw runtime_error("couldn't find '\r\n'");
	}
	string value = content.substr(0, crlfPos);
	content.erase(content.begin(), content.begin() + crlfPos + 2);

	header[key] = value;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HTTP Reponse Builders ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
HttpResponse createHttpResponse(int code, const string& path)
{
	HttpResponse res;

	res.statusCode = code;
	res.header["Date"] = getDateString();
	res.header["Server"] = SERVER_SOFTWARE;

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
		res.header["Content-Length"] = toString(res.body.size());
		res.header["Content-Type"] = getMimeType(getFileExtension(path));
		//res.header["Set-Cookie"] = "username=darren";
	}
	catch (...) {
		std::ifstream file(DEFAULT_404);
		res.body = file.is_open() ? getFileContent(DEFAULT_404) : "404 Not found";
		res.header["Content-Type"] = file.is_open() ? "text/html" : "text/plain";
		res.header["Content-Length"] = toString(res.body.length());
		file.close();
	}
	return res;
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse createHttpResponse(const string& str)
{
	HttpResponse res;

	res.statusCode = 200;
	res.body = str;
	res.header["Date"] = getDateString();
	res.header["Server"] = SERVER_SOFTWARE;
	res.header["Content-Length"] = toString(res.body.size());
	res.header["Content-Type"] = "text/html";

	return res;
}

/* ---------------------------------------------------------------------------------------------- */
HttpResponse createAutoindex(const string& path)
{
	string body("<!DOCTYPE html>");

	body.append("<html><head><title>Directory Index</title>");
	body.append("<link rel=\"stylesheet\" type=\"text/css\" ");
	body.append("href=\"/style/autoindex.css\"></head>");
	body.append("<body><div class=\"container\"><h1 class=\"heading\">");
	body.append("Directory Autoindex</h1><ul class=\"list\">");

	DIR* dir;
	dirent* entry;
	if ((dir = opendir(path.c_str())) == NULL) {
		throw runtime_error("opendir failed");
	}

	while ((entry = readdir(dir)) != NULL) {
		string name(entry->d_name);
		if (name == "." || name == "..") {
			continue;
		}

		Stat fileInfo;
		string filePath = fullPath(path, name);
		stat(filePath.c_str(), &fileInfo);
		name = S_ISDIR(fileInfo.st_mode) ? name + "/" : name;
		// add a href?
		body.append("<li class=\"list-item\"><div class=\"name\">");
		body.append(name).append("</div></li>");
	}
	closedir(dir);
	body.append("</ul></div></body></html>");

	return createHttpResponse(body);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ HTTP Response Utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
string getStatusText(int code)
{
	switch(code) {
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 203: return "Non-Authoritative Information";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 307: return "Temporary Redirect";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		default: return "Unknown";
	}
}

/* --------------------------------------------------------------------------------------------- */
string getMimeType(string ext)
{
	if (ext == ".aac") return "audio/aac";
	if (ext == ".css") return "text/css";
	if (ext == ".csv") return "text/csv";
	if (ext == ".gif") return "image/git";
	if (ext == ".ico") return "image/vnd.microsoft.icon";
	if (ext == ".mp3") return "audio/mpeg";
	if (ext == ".mp4") return "video/mp4";
	if (ext == ".png") return "image/png";
	if (ext == ".pdf") return "application/pdf";
	if (ext == ".php") return "application/x-httpd-php";
	if (ext == ".svg") return "image/svg+xml";
	if (ext == ".txt") return "text/plain";
	if (ext == ".wav") return "audio/wav";
	if (ext == ".mpeg") return "video/mpeg";
	if (ext == ".webp") return "image/webp";
	if (ext == ".json") return "application/json";
	if (ext == ".htm" || ext == ".html") return "text/html";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".mjs" || ext == ".js") return "text/javascript";

	return "text/plain";
}

/* --------------------------------------------------------------------------------------------- */
string getDateString(void)
{
	time_t t = std::time(NULL);
	std::tm* timePtr = std::gmtime(&t);

	char buffer[50];
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timePtr);
	return string(buffer);
}

/* --------------------------------------------------------------------------------------------- */
string toString(HttpResponse res)
{
	string str;
	str.append(string(HTTP_VERSION) + " ");
	str.append(toString(res.statusCode) + " ");
	str.append(getStatusText(res.statusCode) + "\r\n");

	StringMap::iterator it;
	for (it = res.header.begin(); it != res.header.end(); it++) {
		str.append(it->first + ": ").append(it->second + "\r\n");
	}
	str.append("\r\n").append(res.body);
	return str;
}
