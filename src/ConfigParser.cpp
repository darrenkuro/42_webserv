#include "ConfigParser.hpp"
#include <iostream>
/* -------------------------------------------------------------------------- *
 * Constants declaractions and checkers for valid keys in the config.
 * -------------------------------------------------------------------------- */
#pragma region constants

const std::string serverKeyArray[] = {
	"listen", "server_name", "error_page", "client_max_body_size", "location"
};

const std::string locationKeyArray[] = {
	"allowed_method", "return", "alias", "autoindex", "index"
};

// TODO:CHECK THESE!?
const int errorCode[] = {400, 403, 404, 405, 408, 411, 413, 500, 501, 505};

const std::vector<std::string> ConfigParser::validServerKeys(
	serverKeyArray, serverKeyArray +
	sizeof(serverKeyArray) / sizeof(serverKeyArray[0])
);

const std::vector<std::string> ConfigParser::validLocationKeys(
	locationKeyArray, locationKeyArray +
	sizeof(locationKeyArray) / sizeof(locationKeyArray[0])
);

const std::vector<int> ConfigParser::validErrorCodes(errorCode,
	errorCode + sizeof(errorCode) / sizeof(errorCode[0])
);

/* -------------------------------------------------------------------------- */
bool ConfigParser::isValidServerKey(std::string key)
{
	return std::find(validServerKeys.begin(), validServerKeys.end(), key)
			!= validServerKeys.end();
}

bool ConfigParser::isValidLocationKey(std::string key)
{
	return std::find(validLocationKeys.begin(), validLocationKeys.end(), key)
			!= validLocationKeys.end();
}

bool ConfigParser::isValidErrorCode(int code)
{
	return std::find(validErrorCodes.begin(), validErrorCodes.end(), code)
			!= validErrorCodes.end();
}

#pragma endregion

/* -------------------------------------------------------------------------- */
std::vector<ServerConfig> ConfigParser::parse(const std::string& filename)
{
	std::string content = getFileContent(filename);
	lex(content, " \t\n", "{};");

	m_configs.clear();
	while (!m_tokens.empty()) {
		parseServer();
	}

	return m_configs;
}

void ConfigParser::lex (const std::string& content, const std::string& whitespace,
						const std::string& symbol)
{
	m_tokens.clear();

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

std::string ConfigParser::accept(void)
{
	if (m_tokens.empty())
		throw std::runtime_error("Parser: syntax error!");

	std::string token = m_tokens.front();
	m_tokens.pop_front();
	return token;
}

void ConfigParser::consume(const std::string& token)
{
	if (m_tokens.front() != token)
		throw std::runtime_error("can't find " + token + "!");

	m_tokens.pop_front();
}

/* -------------------------------------------------------------------------- */
void ConfigParser::parseServer(void)
{
	ServerConfig server = createServer();
	consume("server"), consume("{");

	std::string token;
	while ((token = accept()) != "}") {
		if (!isValidServerKey(token))
			throw std::runtime_error("Parser: unknown server key " + token + "!");
		if (token == "listen")
			parseAddress(server);
		if (token == "server_name")
			parseServerName(server);
		if (token == "error_page")
			parseErrorPage(server);
		if (token == "location")
			parseLocation(server);
		if (token == "client_max_body_size")
			parseClientMaxBodySize(server);
	}

	// Fill in the default error pages and '/' location if not provided
	addDefaultErrorPages(server);
	addDefaultLocation(server);

	m_configs.push_back(server);
}

void ConfigParser::parseLocation(ServerConfig& server)
{
	LocationConfig location = createLocation();
	parseUri(location), consume("{");

	std::string token;
	while ((token = accept()) != "}") {
		if (!isValidLocationKey(token))
			throw std::runtime_error("Parser: unknown location key " + token + "!");
		if (token == "allowed_method")
			parseAllowedMethods(location);
		if (token == "return")
			parseRedirect(location);
		if (token == "alias")
			parseAlias(location);
		if (token == "autoindex")
			parseAutoindex(location);
		if (token == "index")
			parseIndex(location);
	}
	server.locations.push_back(location);
}


//------------------------------------------------------------------------------
void ConfigParser::parseServerName(ServerConfig& server)
{
	server.serverName = accept();
	consume(";");
}

//------------------------------------------------------------------------------
void ConfigParser::parseAddress(ServerConfig& server)
{
	try {
		std::string token = accept();

		// Resolve host portion
		size_t colonPos = token.find(":");
		if (colonPos != std::string::npos) {
			server.address.host = toIPv4(token.substr(0, colonPos));
			token.erase(token.begin(), token.begin() + colonPos + 1);
		} else {
			server.address.host = 0;
		}

		// Resolve port portion
		int port = toInt(token);
		if (port <= 0 || port > 65535)
			throw std::exception();
		server.address.port = port;

		consume(";");
	}
	catch (...) {
		throw std::runtime_error("Parser: invalid listen!");
	}
}

//------------------------------------------------------------------------------
void ConfigParser::parseErrorPage(ServerConfig& server)
{
	std::vector<std::string> tokens;
	std::string token;

	while ((token = accept()) != ";") {
		tokens.push_back(token);
	}
	if (tokens.size() < 2)
		throw std::runtime_error("Parser: syntax error!");

	// Excluding the last element which is the page path
	try {
		for (size_t i = 0; i < tokens.size() - 1; i++) {
			int code = toInt(tokens[i]);

			// Check if the key already exist or the code isn't used
			std::map<int, std::string>::iterator it;
			if (server.errorPages.find(code) != server.errorPages.end())
				throw std::runtime_error("duplicated error code");
			if (!isValidErrorCode(code))
				throw std::runtime_error("code " + toString(code) + " isn't used");

			server.errorPages[code] = tokens.back();
		}
	}
	catch (std::exception& e) {
		throw std::runtime_error("Parser: error page, " + std::string(e.what()) + "!");
	}
}

//------------------------------------------------------------------------------
void ConfigParser::parseClientMaxBodySize(ServerConfig& server)
{
	try {
		int value = toInt(accept());
		if (value < 0)
			throw std::runtime_error("negative value");
		server.clientMaxBodySize = value;
		consume(";");
	}
	catch (std::exception& e) {
		throw std::runtime_error("Parser: body size, " + std::string(e.what()) + "!");
	}
}


//------------------------------------------------------------------------------
void ConfigParser::parseUri(LocationConfig& location)
{
	location.uri = accept();
	// validate, it has to start with '/'?
	// any illegal characters in the uri?
}


//------------------------------------------------------------------------------
void ConfigParser::parseAllowedMethods(LocationConfig& location)
{
	std::string token;
	while ((token = accept()) != ";") {
		// Define all allowed methods somewhere else in an array?
		// Potentially more and need to be accessible in other places
		if (token != "GET" && token != "POST" && token != "DELETE")
			throw std::runtime_error("Parser: unknown method " + token);

		location.allowedMethods.push_back(token);
	}
}


//------------------------------------------------------------------------------
void ConfigParser::parseRedirect(LocationConfig& location)
{
	try {
		location.redirect.first = toInt(accept());
		location.redirect.second = accept(); // need to validate as url?
		consume(";");
	}
	catch (...) {
		// throw errors
	}
}

//------------------------------------------------------------------------------
void ConfigParser::parseAlias(LocationConfig& location)
{
	try {
		std::string token = accept();
		std::string path = fullPath(ROOT, token);

		// Check if alias is accessible and is a directory
		struct stat pathInfo;
		if (stat(path.c_str(), &pathInfo) != 0 || !S_ISDIR(pathInfo.st_mode))
			throw std::runtime_error("Parser: invalid alias " + token + "!");

		location.alias = token; // path or alias?
		consume(";");
	}
	catch (...) {

	}
	// should it return actually path or just the alias?
}

//------------------------------------------------------------------------------
void ConfigParser::parseAutoindex(LocationConfig& location)
{
	std::string token = accept();
	if (token != "on" && token != "off")
		throw std::runtime_error("Parser: invalid autoindex value!");

	if (token == "on")
		location.autoindex = true;
	if (token == "off")
		location.autoindex = false;
	consume(";");
}


//------------------------------------------------------------------------------
void ConfigParser::parseIndex(LocationConfig& location)
{
	std::string token;
	while ((token = accept()) != ";") {
		location.index.push_back(token);
	}
}

//------------------------------------------------------------------------------
ServerConfig ConfigParser::createServer(void)
{
	ServerConfig config;

	config.address.host = 0;
	config.address.port = 80;
	config.root = ROOT;
	config.clientMaxBodySize = -1;

	return config;
}

void ConfigParser::addDefaultErrorPages(ServerConfig& server)
{
	for (size_t i = 0; i < validErrorCodes.size(); i++) {
		int code = validErrorCodes[i];
		if (server.errorPages.find(code) == server.errorPages.end())
			server.errorPages[code] = "/default_error/" + toString(code) + ".html";
	}
}

// make two different methods, one init?
LocationConfig ConfigParser::createLocation(void)
{
	LocationConfig location;

	// Set the default values

	// location.allowedMethods.push_back("GET");
	location.autoindex = false;
	location.alias = "";
	location.redirect.first = 0;
	location.redirect.second = "";

	return location;
}

void ConfigParser::addDefaultLocation(ServerConfig& server)
{
	// Check if the default location already exist
	std::vector<LocationConfig>::iterator it;
	for (it = server.locations.begin(); it != server.locations.end(); it++) {
		if (it->uri == "/")
			return;
	}

	LocationConfig location = createLocation();
	location.uri = "/";
	location.index.push_back("index.html");

	server.locations.push_back(location);
}
