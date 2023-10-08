#include "ConfigParser.hpp"

#include <algorithm>		// find

#include "utils.hpp"		// fullPath, toInt, toIpNum

/* ============================================================================================== */
/*                                                                                                */
/*                               ConfigParser Class Implementation                                */
/*                                                                                                */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
const string serverKeyArray[] = {
	"root","server_name",  "listen", "client_max_body_size", "error_page", "location"
};

const string locationKeyArray[] = {
	"autoindex", "alias", "allowed_method", "index", "return"
};

const int errorCode[] = {400, 403, 404, 405, 408, 411, 413, 500, 501, 505};

const int redirectCode[] = {301, 302, 303, 307, 308};

const vector<string> ConfigParser::validServerKeys(
	serverKeyArray, serverKeyArray + sizeof(serverKeyArray) / sizeof(serverKeyArray[0])
);

const vector<string> ConfigParser::validLocationKeys(
	locationKeyArray, locationKeyArray + sizeof(locationKeyArray) / sizeof(locationKeyArray[0])
);

const vector<int> ConfigParser::validErrorCodes(errorCode,
	errorCode + sizeof(errorCode) / sizeof(errorCode[0])
);

const vector<int> ConfigParser::validRedirectCodes(redirectCode,
	redirectCode + sizeof(redirectCode) / sizeof(redirectCode[0])
);

/* ---------------------------------------------------------------------------------------------- */
bool ConfigParser::isValidServerKey(string key)
{
	return std::find(validServerKeys.begin(), validServerKeys.end(), key)
				!= validServerKeys.end();
}

bool ConfigParser::isValidLocationKey(string key)
{
	return std::find(validLocationKeys.begin(), validLocationKeys.end(), key)
				!= validLocationKeys.end();
}

bool ConfigParser::isValidErrorCode(int code)
{
	return std::find(validErrorCodes.begin(), validErrorCodes.end(), code)
				!= validErrorCodes.end();
}

bool ConfigParser::isValidRedirectCode(int code)
{
	return std::find(validRedirectCodes.begin(), validRedirectCodes.end(), code)
				!= validRedirectCodes.end();
}

/* ---------------------------------------------------------------------------------------------- */
vector<ServerConfig> ConfigParser::parse(const string& filename)
{
	string content = getFileContent(filename);
	lex(content, " \t\n", "{};");
	m_configs.clear();

	// Config file need to have at least one server
	do {
		try {
			parseServer();
		}
		catch (const exception& e) {
			throw runtime_error("Parser: " + string(e.what()) + "!");
		}
	} while (!m_tokens.empty());

	return m_configs;
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::lex(const string& content, const string& whitespace, const string& symbol)
{
	m_tokens.clear();

	size_t pos = 0;
	while (pos < content.size()) {
		if (symbol.find(content[pos]) != string::npos) {
			m_tokens.push_back(string(1, content[pos]));
			pos++;
		}
		else if (whitespace.find(content[pos]) != string::npos) {
			pos++;
		}
		else {
			size_t tokEnd = content.find_first_of(whitespace + symbol, pos);
			if (tokEnd == string::npos)
				tokEnd = content.size();
			m_tokens.push_back(content.substr(pos, tokEnd - pos));
			pos += m_tokens.back().size();
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */
string ConfigParser::accept(void)
{
	if (m_tokens.empty()) {
		throw runtime_error("syntax error");
	}

	string token = m_tokens.front();
	m_tokens.pop_front();
	return token;
}

void ConfigParser::consume(const string& token)
{
	if (m_tokens.front() != token) {
		throw runtime_error("syntax error");
	}
	m_tokens.pop_front();
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseServer(void)
{
	ServerConfig server = createServer();
	consume("server"), consume("{");

	string token;
	while ((token = accept()) != "}") {
		if (!isValidServerKey(token)) {
			throw runtime_error("unknown server key " + token);
		}
		if (token == "root") {
			parseRoot(server);
		}
		if (token == "server_name") {
			parseServerName(server);
		}
		if (token == "listen") {
			parseAddress(server);
		}
		if (token == "client_max_body_size") {
			parseClientMaxBodySize(server);
		}
		if (token == "error_page") {
			parseErrorPage(server);
		}
		if (token == "location") {
			parseLocation(server);
		}
	}

	// Check server include the required fields
	if (!server.address.ip && !server.address.port) {
		throw runtime_error("server has no listen field");
	}

	// Fill in the default error pages and '/' location if not provided
	addDefaultErrorPages(server);
	addDefaultLocation(server);

	m_configs.push_back(server);
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseLocation(ServerConfig& server)
{
	LocationConfig location = createLocation();
	parseUri(location), consume("{");

	// Check if location uri already exists
	vector<LocationConfig>::iterator it;
	for (it = server.locations.begin(); it != server.locations.end(); it++) {
		if (it->uri == location.uri)
			throw runtime_error("duplication location " + it->uri);
	}

	string token;
	while ((token = accept()) != "}") {
		if (!isValidLocationKey(token)) {
			throw runtime_error("unknown location key " + token);
		}
		if (token == "autoindex") {
			parseAutoindex(location);
		}
		if (token == "alias") {
			parseAlias(location);
		}
		if (token == "allowed_method") {
			parseAllowedMethods(location);
		}
		if (token == "index") {
			parseIndex(location);
		}
		if (token == "return") {
			parseRedirect(location);
		}
	}

	// Add default method (GET) if no allowed method is specified
	if (location.allowedMethods.empty()) {
		location.allowedMethods.push_back("GET");
	}

	// Check for required fields of locaiton context block?

	server.locations.push_back(location);
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseRoot(ServerConfig& server)
{
	server.root = fullPath(ROOT, accept());
	consume(";");
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseServerName(ServerConfig& server)
{
	server.serverName = accept();
	consume(";");
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseAddress(ServerConfig& server)
{
	try {
		string token = accept();

		// Resolve host portion
		size_t colonPos = token.find(":");
		if (colonPos != string::npos) {
			server.address.ip = toIpNum(token.substr(0, colonPos));
			token.erase(token.begin(), token.begin() + colonPos + 1);
		}
		else {
			server.address.ip = 0;
		}

		// Resolve port portion
		int port = toInt(token);
		if (port <= 0 || port > 65535) {
			throw runtime_error("port out of range");
		}
		server.address.port = port;

		consume(";");
	}
	catch (const exception& e) {
		throw runtime_error("invalid listen - " + string(e.what()));
	}
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseClientMaxBodySize(ServerConfig& server)
{
	try {
		int value = toInt(accept());
		if (value < 0) {
			throw runtime_error("negative");
		}
		server.clientMaxBodySize = value;
		consume(";");
	}
	catch (const exception& e) {
		throw runtime_error("invalid max body size - " + string(e.what()));
	}
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseErrorPage(ServerConfig& server)
{
	vector<string> tokens;
	string token;

	while ((token = accept()) != ";") {
		tokens.push_back(token);
	}
	if (tokens.size() < 2) {
		throw runtime_error("syntax error");
	}

	// Excluding the last element which is the page path
	try {
		for (size_t i = 0; i < tokens.size() - 1; i++) {
			int code = toInt(tokens[i]);

			// Check if the key already exist or the code isn't used
			map<int, string>::iterator it;
			if (server.errorPages.find(code) != server.errorPages.end()) {
				throw runtime_error("duplicated code");
			}
			if (!isValidErrorCode(code)) {
				throw runtime_error("invalid code " + toString(code));
			}

			server.errorPages[code] = tokens.back();
		}
	}
	catch (const exception& e) {
		throw runtime_error("invalid error page - " + string(e.what()));
	}
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::parseUri(LocationConfig& location)
{
	location.uri = accept();
}

void ConfigParser::parseAutoindex(LocationConfig& location)
{
	string token = accept();
	if (token != "on" && token != "off") {
		throw runtime_error("invalid autoindex");
	}
	location.autoindex = token == "on" ? true : false;
	consume(";");
}

void ConfigParser::parseAlias(LocationConfig& location)
{
	string token = accept();

	location.alias = token;
	consume(";");
}

void ConfigParser::parseAllowedMethods(LocationConfig& location)
{
	string token;
	while ((token = accept()) != ";") {
		if (token != "GET" && token != "POST" && token != "DELETE") {
			throw runtime_error("unknown method " + token);
		}

		location.allowedMethods.push_back(token);
	}
}

void ConfigParser::parseIndex(LocationConfig& location)
{
	string token;
	while ((token = accept()) != ";") {
		location.index.push_back(token);
	}
}

void ConfigParser::parseRedirect(LocationConfig& location)
{
	location.redirect.first = toInt(accept());
	if (!isValidRedirectCode(location.redirect.first)) {
		throw runtime_error("invalid redirect code");
	}
	location.redirect.second = accept();
	consume(";");
}

/* ---------------------------------------------------------------------------------------------- */
ServerConfig ConfigParser::createServer(void)
{
	ServerConfig config;

	config.address.ip = 0;
	config.address.port = 0;
	config.root = ROOT;
	config.clientMaxBodySize = -1;

	return config;
}

/* ---------------------------------------------------------------------------------------------- */
LocationConfig ConfigParser::createLocation(void)
{
	LocationConfig location;

	// Set the default values
	location.autoindex = false;
	location.alias = "";
	location.redirect.first = 0;
	location.redirect.second = "";

	return location;
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::addDefaultErrorPages(ServerConfig& server)
{
	for (size_t i = 0; i < validErrorCodes.size(); i++) {
		int code = validErrorCodes[i];
		if (server.errorPages.find(code) == server.errorPages.end()) {
			server.errorPages[code] = "/default_error/" + toString(code) + ".html";
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */
void ConfigParser::addDefaultLocation(ServerConfig& server)
{
	// Check if the default location already exist
	vector<LocationConfig>::iterator it;
	for (it = server.locations.begin(); it != server.locations.end(); it++) {
		if (it->uri == "/") {
			return;
		}
	}

	// Should add one default or throw an error??
	LocationConfig location = createLocation();
	location.uri = "/";
	location.allowedMethods.push_back("GET");
	location.index.push_back("index.html"); // default should be added in places where there is none as well

	server.locations.push_back(location);
}
