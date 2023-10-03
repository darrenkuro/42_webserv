#include "utils.hpp"
#include "ConfigParser.hpp"

/* --------------------------------------------------------------------------------------------- */
string getFileContent(string path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw runtime_error("Failed to open file: " + path);
	}

	string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

/* --------------------------------------------------------------------------------------------- */
string toString(int value)
{
	std::stringstream ss;
	ss << value;

	// Unlikely to happen, bad alloc only
	if (ss.fail()) return "";

	return ss.str();
}

int toInt(string str)
{
	std::istringstream iss(str);
	int result;
	char remainingChar;

	// Check if there are leftover characters
	if (!(iss >> result) || (iss.get(remainingChar))) {
		throw runtime_error("convert " + str + " toInt fails");
	}

	// Check fail and edge cases
	if (result == 0 && str != "0" && str != "+0" && str != "-0") {
		throw runtime_error("convert " + str + " toInt fails");
	}
	if (result == INT_MAX && str != "2147483647" && str != "+2147483647") {
		throw runtime_error("convert " + str + " toInt fails");
	}
	if (result == INT_MIN && str != "-2147483648") {
		throw runtime_error("convert " + str + " toInt fails");
	}

	return result;
}

std::string toIPString(in_addr_t ip)
{
	std::ostringstream oss;
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&ip);

	oss << static_cast<int>(bytes[0]) << '.'
		<< static_cast<int>(bytes[1]) << '.'
		<< static_cast<int>(bytes[2]) << '.'
		<< static_cast<int>(bytes[3]);

	return oss.str();
}

in_addr_t toIPv4(string str)
{
	in_addr_t result = 0;

	for (int i = 0; i < 4; i++) {
		if (i < 3 && str.find('.') == string::npos) {
			throw runtime_error("failed to convert " + str);
		}

		string token = i < 3 ? str.substr(0, str.find('.')) : str;
		int value = toInt(token);
		if (!isAllDigit(token) || value < 0 || value > 255) {
			throw runtime_error("failed to convert " + str);
		}

		result = (result << 8) | value;

		if (i < 3)
			str.erase(str.begin(), str.begin() + str.find('.') + 1);
	}

	return htonl(result);
}

/* --------------------------------------------------------------------------------------------- */
bool isAllDigit(string str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i])) {
			return false;
		}
	}
	return true;
}

string fullPath(string root, string path)
{

	try {
		// Remove '/' from root and path
		root = root.at(root.size() - 1) == '/' ? root.substr(0, root.size() - 1) : root;
		path = path.at(0) == '/' ? path.substr(1) : path;
	}
	catch (...) {
		// Protect against if strings are empty or index out of bound
		return "";
	}
	return root + "/" + path;
}

string getExtension(string path)
{
	size_t pos = path.find_last_of('.');

	if (pos != string::npos && pos != 0) {
		return path.substr(pos);
	}

	return "";
}

Address getAddressFromFd(int fd)
{
	struct sockaddr_in serverAddress;
	socklen_t addrLen = sizeof(serverAddress);
	if (getsockname(fd, (struct sockaddr*)&serverAddress, &addrLen) == -1) {
		throw runtime_error("getsockname() failed");
	}

	Address addr;
	addr.ip = ntohl(serverAddress.sin_addr.s_addr);
	addr.port = ntohs(serverAddress.sin_port);
	log(INFO, "HH %d", addr.port);
	return addr;
}

/* --------------------------------------------------------------------------------------------- */
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
string toString(HttpResponse res)
{
	string str;
	str.append(string(HTTP_VERSION) + " ");
	str.append(toString(res.statusCode) + " ");
	str.append(getStatusText(res.statusCode) + "\r\n");

	map<string, string>::iterator it;
	for (it = res.header.begin(); it != res.header.end(); it++) {
		str.append(it->first + ": ").append(it->second + "\r\n");
	}
	str.append("\r\n").append(res.body);
	return str;
}

string getDate(void)
{
	std::time_t t = std::time(NULL);
	std::tm* timePtr = std::gmtime(&t);

	char buffer[50];
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timePtr);
	return string(buffer);
}

ostream& displayTimestamp(ostream& os)
{
	std::time_t result = std::time(NULL);
	string timestamp(std::ctime(&result));
	os << CYAN << "[" + timestamp.substr(11, 8) + "]" << RESET;
	return os;
}

ostream& align(ostream& os)
{
	string padding(PADDING, ' ');
	os << padding;
	return os;
}

void displayLogLevel(int level)
{
	switch (level)
	{
	case DEBUG:
		cout << YELLOW << "[DEBUG]" << RESET << "   ";
		break;

	case INFO:
		cout << CYAN << "[INFO]" << RESET << "    ";
		break;

	case WARNING:
		cout << RED << "[WARNING]" << RESET << " ";
		break;

	case ERROR:
		cout << RED << "[ERROR]" << RESET << "   ";
		break;
	}
}

void log(int level, const char* format, ...)
{
	if (level > LOG_DISPLAY_LEVEL) {
		return;
	}
	cout << displayTimestamp << " ";
	displayLogLevel(level);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	cout << endl;
}

void log(ServerConfig config)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << "ServerConfig: ";
	cout << config;
}

void log(HttpRequest req, int clientID)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << ORANGE;
	cout << "HTTP Req>> Client[ID " << clientID << "]" << endl;
	cout << req << RESET << endl;
}

void log(HttpResponse res, int clientID)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << ORANGE;
	cout << "HTTP <<Res Client[ID " << clientID << "]" << endl;
	cout << res << RESET << endl;
}

ostream& operator<<(ostream& os, HttpRequest req)
{
	os << align;
	(void)req;
	os << "Method[" << req.method << "] ";
	os << "Uri[" << req.uri << "] ";
	os << "Version[" << req.version << "] ";
	os << "Host[" << req.header.find("Host")->second << "] ";
	return os;
}

ostream& operator<<(ostream& os, HttpResponse res)
{
	os << align;
	os << "Status[" << res.statusCode << "]";
	return os;
}

ostream &operator<<(ostream &os, const Address address)
{
	os << "Address[" << toIPString(address.ip) << ":";
	os << address.port << "]";
	return os;
}

ostream &operator<<(ostream &os, const LocationConfig location)
{
	os << "URI[" << location.uri << "] ";
	os << "Alias[" << location.alias << "] ";
	os << "Autoindex[" << location.autoindex << "] ";
	os << "Redirect[" << location.redirect.first << ":";
	os << location.redirect.second << "] AllowedMethods[";

	vector<string>::const_iterator it;
	for (it = location.allowedMethods.begin(); it != location.allowedMethods.end(); it++) {
		os << " " << *it;
	}
	os << " ] Index[";
	for (it = location.index.begin(); it != location.index.end(); it++) {
			os << " " << *it;
	}
	os << " ]";
	return os;
}

ostream &operator<<(ostream &os, const ServerConfig config)
{
	os << "ServerName[" << config.serverName << "] ";
	os << "Root[" << config.root << "] ";
	os << "MaxBodySize[" << config.clientMaxBodySize << "] ";
	os << config.address << endl;

	os << align, os << "Locations: " << endl;
	vector<LocationConfig>::const_iterator it2;
	for (it2 = config.locations.begin(); it2 != config.locations.end(); it2++) {
		os << align;
		os << ">> " << *it2 << endl;
	}

	os << align, os << "ErrorPages: ";
	map<int, string>::const_iterator it;
	int i = 0;
	for (it = config.errorPages.begin(); it != config.errorPages.end(); it++) {
		if (i++ % 3 == 0)
			os << endl, os << align, os << ">> ";
		os << it->first << "[" << it->second << "] ";
	}
	os << endl;
	return os;
}

HttpResponse createBasicResponse(int code, string path)
{
	HttpResponse res;

	res.statusCode = code;
	res.header["Date"] = getDate();
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
		res.header["Content-Length"] = toString(res.body.length());
		res.header["Content-Type"] = getMimeType(getExtension(path));
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

timeval getTime(void)
{
	timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime;
}
