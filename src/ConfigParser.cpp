#include "ConfigParser.hpp"

const std::string serverKeyArray[] = {
	"listen", "server_name", "index",
	"error_page", "client_max_body_size", "location"
};

const std::string locationKeyArray[] = {
	"allowed_method", "return", "alias", "autoindex", "index"
};

const int errorCode[] = {400, 401, 403, 404, 500, 502, 503};

const std::vector<int> ConfigParser::validErrorCodes(errorCode, errorCode + sizeof(errorCode) / sizeof(errorCode[0]));

const std::vector<std::string> ConfigParser::validServerKeys(
    serverKeyArray, serverKeyArray + sizeof(serverKeyArray) / sizeof(serverKeyArray[0])
);

const std::vector<std::string> ConfigParser::validLocationKeys(
    locationKeyArray, locationKeyArray + sizeof(locationKeyArray) / sizeof(locationKeyArray[0])
);

//------------------------------------------------------------------------------
std::vector<ServerConfig> ConfigParser::parse(const std::string filename)
{
	std::string content = getFileContent(filename);
	lex(content, " \t\n", "{};");

	// std::deque<std::string>::iterator it;
	// for (it = m_tokens.begin(); it != m_tokens.end(); it++)
	// 	std::cout << *it << std::endl;

	std::vector<ServerConfig> configs;
	while (!m_tokens.empty()) {
		configs.push_back(parseServer());
	}
	return configs;
}

void ConfigParser::lex(std::string content, std::string whitespace, std::string symbol)
{
	size_t pos = 0;
	while (pos < content.size()) {
		if (symbol.find(content[pos]) != std::string::npos) {
			m_tokens.push_back(std::string(1, content[pos]));
			pos++;
		}
		else if (whitespace.find(content[pos]) != std::string::npos) {
			pos++;
		}
		else {
			size_t tokEnd = content.find_first_of(whitespace + symbol, pos);
			if (tokEnd == std::string::npos)
				tokEnd = content.size();
			m_tokens.push_back(content.substr(pos, tokEnd - pos));
			pos += m_tokens.back().size();
		}
	}
}

//------------------------------------------------------------------------------
ServerConfig ConfigParser::parseServer()
{
	ServerConfig config = defaultServer();
	consume("server");
	consume("{");
	std::string token;
	while ((token = accept()) != "}") {
		if (!isValidServerKey(token)) {
			throw std::runtime_error("Parser: unknown server key " + token + "!");
		}
		else if (token == "listen") {
			config.address = parseAddress();
		}
		else if (token == "server_name") {
			config.serverName = parseServerName();
		}
		else if (token == "index") {
			config.index = parseIndex();
		}
		else if (token == "error_page") {
			config.errorPages.insert(parseErrorPage());
		}
		else if (token == "client_max_body_size") {
			config.clientMaxBodySize = parseClientMaxBodySize();
		}
		else if (token == "location") {
			config.locations.push_back(parseLocation());
		}
	}
	return config;
}

//------------------------------------------------------------------------------
LocationConfig ConfigParser::parseLocation(void)
{
	LocationConfig location = defaultLocation();
	location.uri = parseUri();
	consume("{");
	std::string token;
	while ((token = accept()) != "}") {
		if (!isValidLocationKey(token)) {
			throw std::runtime_error("Parser: unknown location key " + token + "!");
		}
		else if (token == "allowed_method") {
			location.allowedMethods = parseAllowedMethods();
		}
		else if (token == "return") {
			location.redirect = parseRedirect();
		}
		else if (token == "alias") {
			location.alias = parseAlias();
		}
		else if (token == "autoindex") {
			location.autoindex = parseAutoindex();
		}
		else if (token == "index") {
			location.index = parseIndex();
		}
	}
	return location;
}


//------------------------------------------------------------------------------
std::string ConfigParser::parseServerName(void)
{
	std::string token = accept();
	// validate servername?
	consume(";");
	return token;
}

//------------------------------------------------------------------------------
SocketAddress ConfigParser::parseAddress(void)
{
	SocketAddress address;
	std::string token = accept();
	if (token.find(":") != std::string::npos) {
		// handle the host portion, can't use inet_pton? (not in allowed functions)
		// write one from scratch, in utils?
		// also need to convert to network endianness?
		token.erase(token.begin(), token.begin() + token.find(":") + 1);
	} else {
		address.host = 0;
	}

	try {
		int port = toInt(token);
		if (port <= 0 || port > 65535) {
			throw std::exception();
		}
		address.port = port;
	}
	catch (std::exception &e) {
		throw std::runtime_error("Parser: invalid port value!");
	}
	consume(";");
	return address;
}


//------------------------------------------------------------------------------
std::pair<int, std::string> ConfigParser::parseErrorPage(void)
{
	std::pair<int, std::string> errorPage;
	std::string errorCode = accept();
	// could accept multiple all at once
	try {
		if (!isAllDigit(errorCode)) {
			throw std::exception();
		}
		errorPage.first = toInt(errorCode);
		if (!isValidErrorCode(errorPage.first)) {
			throw std::exception();
		}
	}
	catch (std::exception &e) {
		throw std::runtime_error("Parser: invalid error code " + errorCode + "!");
	}

	errorPage.second = accept();
	try {
		// what if it doesn't exist?
		// could root be changed also or should it do a check on file existence here?
	}
	catch (std::exception &e) {
		throw std::runtime_error("Parser: invalid error page " + errorPage.second + "!");
	}
	consume(";");
		// keep accepting until you see the token;
	return errorPage;
}

//------------------------------------------------------------------------------
size_t ConfigParser::parseClientMaxBodySize(void)
{
	// const std::string unitChar = "kKmM";
	// std::string token = accept();
	// int unit = 1;
	// if (unitChar.find(token) != std::string::npos) {
	// 	if (token == "k" || token == "K") {
	// 		unit = 1000;
	// 	}
	// 	else if (token == "m" || token == "M") {
	// 		unit = 1000000;
	// 	}
	// 	token.erase(token.length() - 1);
	// }

	std::string token = accept();
	int value;
	try {
		value = toInt(token);
	}
	catch (std::exception &e) {
		throw std::runtime_error("Parser: invalid client max body size!");
	}
	consume(";");
	return value;
}


//------------------------------------------------------------------------------
std::string ConfigParser::parseUri(void)
{
	std::string token = accept();
	// validate, it has to start with '/'?
	// any illegal characters in the uri?
	return token;
}


//------------------------------------------------------------------------------
std::vector<std::string> ConfigParser::parseAllowedMethods(void)
{
	std::vector<std::string> allowedMethods;
	std::string token;
	while ((token = accept()) != ";") {
		// Define all allowed methods somewhere else in an array?
		// Potentially more and need to be accessible in other places
		if (token != "GET" && token != "POST" && token != "DELETE") {
			throw std::runtime_error("Parser: unknown method " + token);
		}
		allowedMethods.push_back(token);
	}
	return allowedMethods;
}


//------------------------------------------------------------------------------
std::string ConfigParser::parseRedirect(void)
{
	std::string redirect = accept();
	// return code url?
	consume(";");
	return redirect;
}

//------------------------------------------------------------------------------
std::string ConfigParser::parseAlias(void)
{
	std::string token = accept();
	std::string path = ROOT + token;
	struct stat pathInfo;
	if (stat(path.c_str(), &pathInfo) != 0 || !S_ISDIR(pathInfo.st_mode))
		throw std::runtime_error("[Error] Parser: invalid alias " + token + "!");
	consume(";");
	// should it return actually path or just the alias?
	return token;
}

//------------------------------------------------------------------------------
bool ConfigParser::parseAutoindex(void)
{
	std::string token = accept();
	bool autoindex;
	if (token == "on") {
		autoindex = true;
	}
	else if (token == "off") {
		autoindex = false;
	}
	else {
		throw std::runtime_error("Parser: invalid autoindex value!");
	}
	consume(";");
	return autoindex;
}


//------------------------------------------------------------------------------
std::vector<std::string> ConfigParser::parseIndex(void)
{
	std::vector<std::string> index;
	std::string token;
	while ((token = accept()) != ";") {
		index.push_back(token);
	}
	return index;
}

//------------------------------------------------------------------------------
void ConfigParser::consume(const std::string token)
{
	if (m_tokens.front() != token) {
		throw std::runtime_error("Parser syntax: can't find " + token);
	}
	m_tokens.pop_front();
}

std::string ConfigParser::accept()
{
	std::string token = m_tokens.front();
	m_tokens.pop_front();
	return token;
}


//------------------------------------------------------------------------------
ServerConfig ConfigParser::defaultServer(void)
{
	ServerConfig config;
	config.serverName = "default.com";
	config.address.host = 0;
	config.address.port = 80;
	// default error pages for all the errors required to have
	config.root = ROOT;
	config.errorPages[400] = "/default_error/400.html";
	config.clientMaxBodySize = 100000;
	return config;
}

LocationConfig ConfigParser::defaultLocation(void)
{
	LocationConfig location;
	// Set the default values that are required
	location.autoindex = false;
	return location;
}

//------------------------------------------------------------------------------
bool ConfigParser::isValidServerKey(std::string key)
{
	return std::find(validServerKeys.begin(), validServerKeys.end(), key) != validServerKeys.end();
}

bool ConfigParser::isValidLocationKey(std::string key)
{
	return std::find(validLocationKeys.begin(), validLocationKeys.end(), key) != validLocationKeys.end();
}

bool ConfigParser::isValidErrorCode(int code)
{
	return std::find(validErrorCodes.begin(), validErrorCodes.end(), code) != validErrorCodes.end();
}

bool ConfigParser::isAllDigit(std::string str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i])) {
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const ServerConfig config)
{
	// displayLogLevel(INFO);

	//os << std::string(10, ' ');
	os << "ServerConfig" << std::endl;
	os << "	ServerName: " << config.serverName << std::endl;
	os << "	Root: " << config.root << std::endl;
	os << "	" << config.address;
	os << "	ClientMaxBodySize: " << config.clientMaxBodySize << std::endl;
	os << "	Index: " << std::endl;
	for (std::vector<std::string>::const_iterator it = config.index.begin(); it != config.index.end(); it++) {
		os << "		" << *it << std::endl;
	}
	os << "	ErrorPages: " << std::endl;
	for (std::map<int, std::string>::const_iterator it = config.errorPages.begin(); it != config.errorPages.end(); it++) {
		os << "		" << it->first << " " << it->second << std::endl;
	}
	os << "	Locations: " << std::endl;
	for (std::vector<LocationConfig>::const_iterator it = config.locations.begin(); it != config.locations.end(); it++) {
		os << *it << std::endl;
	}
	return os;
}

std::ostream &operator<<(std::ostream &os, const LocationConfig location)
{
	os << "		URI: " << location.uri << std::endl;
	os << "		Redirect: " << location.redirect << std::endl;
	os << "		Alias: " << location.alias << std::endl;
	os << "		Autoindex: " << location.autoindex << std::endl;
	os << "		Index: " << std::endl;
	for (std::vector<std::string>::const_iterator it = location.index.begin(); it != location.index.end(); it++) {
		os << "			" << *it << std::endl;
	}
	os << "		AllowedMethods: ";
	for (std::vector<std::string>::const_iterator it = location.allowedMethods.begin(); it != location.allowedMethods.end(); it++) {
		os << *it << " ";
	}
	return os;
}

std::ostream &operator<<(std::ostream &os, const SocketAddress address)
{
	os << "Address: [host] " << address.host << " [port] " << address.port << std::endl;
	return os;
}


// std::string ConfigParser::parseRoot(void)
// {
// 	std::string path = accept();
// 	struct stat pathInfo;
// 	if (stat(path.c_str(), &pathInfo) != 0 || !S_ISDIR(pathInfo.st_mode))
// 		throw std::runtime_error("[Error] Parser: invalid root " + path + "!");
// 	consume(";");
// 	return path;
// }
