#include "ConfigParser.hpp"

const std::string serverKeyArray[] = {
	"listen", "server_name", "index",
	"error_page", "client_max_body_size", "location"
};

const std::string locationKeyArray[] = {
	"limit_except", "return", "alias", "autoindex", "index"
};

const std::vector<std::string> ConfigParser::validServerKeys(
    serverKeyArray, serverKeyArray + sizeof(serverKeyArray) / sizeof(serverKeyArray[0])
);

const std::vector<std::string> ConfigParser::validLocationKeys(
    locationKeyArray, locationKeyArray + sizeof(locationKeyArray) / sizeof(locationKeyArray[0])
);

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

ServerConfig ConfigParser::parseServer()
{
	ServerConfig config = defaultServer();
	consume("server");
	consume("{");
	std::string token;
	while ((token = accept()) != "}") {
		if (!isValidServerKey(token))
			throw std::runtime_error("Parser: unknown server key " + token + "!");
		if (token == "listen")
			config.address = parseListen();
		else if (token == "server_name")
			config.serverName = parseServerName();
		else if (token == "error_page")
			config.errorPages.insert(parseErrorPage());
		else if (token == "client_max_body_size")
			config.clientMaxBodySize = parseClientMaxBodySize();
		else if (token == "location")
			config.locations.push_back(parseLocation());
	}
	return config;
}

LocationConfig ConfigParser::parseLocation(void)
{
	LocationConfig location = defaultLocation();
	location.uri = parseUri();
	consume("{");
	std::string token;
	while ((token = accept()) != "}") {
		if (!isValidLocationKey(token))
			throw std::runtime_error("Parser: unknown location key " + token + "!");
		if (token == "autoindex")
			location.autoindex = parseAutoindex();
	}
	return location;
}

std::string ConfigParser::parseServerName(void)
{
	std::string token = accept();
	// validate servername?
	consume(";");
	return token;
}

SocketAddress ConfigParser::parseListen(void)
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
	consume(";");
	if (!isAllDigit(token))
		throw std::runtime_error("Parser: invalid port value!");
	int port = atoi(token.c_str());
	if (port <= 0 || port > 65535)
		throw std::runtime_error("Parser: invalid port value!");
	address.port = port;
	return address;
}

std::pair<int, std::string> ConfigParser::parseErrorPage(void)
{
	std::pair<int, std::string> errorPage;
	std::string errorCode = accept();
	if (!isAllDigit(errorCode))
		throw std::runtime_error("Parser: invalid error code " + errorCode + "!");
	// what happens when it overflows?
	errorPage.first = atoi(errorCode.c_str());
	// check it's valid code: 400/401/403/404/500/502/503 But there might be more

	// what if it doesn't exist?
	// could root be changed also or should it do a check on file existence here?
	errorPage.second = accept();
	consume(";");
	// keep accepting until you see the token;
	return errorPage;
}

size_t ConfigParser::parseClientMaxBodySize(void)
{
	const std::string unitChar = "kKmM";
	std::string token = accept();
	int unit = 1;
	if (unitChar.find(token.back()) != std::string::npos) {
		if (token.back() == 'k' || token.back() == 'K')
			unit = 1000;
		else if (token.back() == 'm' || token.back() == 'M')
			unit = 1000000;
		token.erase(token.length() - 1);
	}
	// check overflow!
	size_t value = 0;
	consume(";");
	return value * unit;
}

std::string ConfigParser::parseUri(void)
{
	std::string token = accept();
	// validate, it has to start with '/'?
	return token;
}

std::vector<std::string> ConfigParser::parseAllowedMethods(void)
{
	std::vector<std::string> allowedMethods;

	return allowedMethods;
}

std::string ConfigParser::parseRedirect(void)
{
	std::string redirect = accept();

	consume(";");
	return redirect;
}

std::string ConfigParser::parseAlias(void)
{
	std::string alias = accept();
	// check if directory exist, move to util helper function
	consume(";");
	return alias;
}


bool ConfigParser::parseAutoindex(void)
{
	std::string token = accept();
	bool autoindex;
	if (token == "on")
		autoindex = true;
	else if (token == "off")
		autoindex = false;
	else
		throw std::runtime_error("Parser: invalid autoindex value!");
	consume(";");
	return autoindex;
}

std::vector<std::string> ConfigParser::parseIndex(void)
{
	std::vector<std::string> index;
	std::string token;
	while ((token = accept()) != ";") {
		index.push_back(token);
	}
	return index;
}

void ConfigParser::consume(const std::string token)
{
	if (m_tokens.front() != token)
		throw std::runtime_error("Parser syntax: can't find " + token);
	m_tokens.pop_front();
}

std::string ConfigParser::accept()
{
	std::string token = m_tokens.front();
	m_tokens.pop_front();
	return token;
}

ServerConfig ConfigParser::defaultServer(void)
{
	ServerConfig config;
	config.serverName = "default.com";
	config.address.host = 0;
	config.address.port = 80;
	// default error pages for all the errors required to have
	config.root = "./public";
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

bool ConfigParser::isValidServerKey(std::string key)
{
	return std::find(validServerKeys.begin(), validServerKeys.end(), key) != validServerKeys.end();
}

bool ConfigParser::isValidLocationKey(std::string key)
{
	return std::find(validLocationKeys.begin(), validLocationKeys.end(), key) != validLocationKeys.end();
}

bool ConfigParser::isAllDigit(std::string str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
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
