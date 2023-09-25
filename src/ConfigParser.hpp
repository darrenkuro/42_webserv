#pragma once

#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <stdexcept>
#include "utils.hpp"

#define ROOT "./public"

struct LocationConfig
{
	bool autoindex;
	std::string uri;
	std::string alias;
	std::vector<std::string> index;
	std::vector<std::string> allowedMethods;
	std::pair<int, std::string> redirect;
};

struct ServerConfig
{
	Address address;
	std::string serverName;
	std::string root;
	ssize_t clientMaxBodySize;
	std::map<int, std::string> errorPages;
	std::vector<LocationConfig> locations;
};

class ConfigParser
{
public:
	std::vector<ServerConfig> parse(const std::string& filename);

private:
	std::vector<ServerConfig> m_configs;
	std::deque<std::string> m_tokens;

	// Logic
	void lex(const std::string& content, const std::string& whitespace,
			 const std::string& symbol);

	std::string accept(void);
	void consume(const std::string& token);

	ServerConfig createServer(void);
	LocationConfig createLocation(void);
	void addDefaultErrorPages(ServerConfig& server);
	void addDefaultLocation(ServerConfig& server);

	// Context & Field Parser
	void parseServer(void);
	void parseLocation(ServerConfig& server);

	void parseRoot(ServerConfig& server);
	void parseAddress(ServerConfig& server);
	void parseServerName(ServerConfig& server);
	void parseErrorPage(ServerConfig& server);
	void parseClientMaxBodySize(ServerConfig& server);

	void parseUri(LocationConfig& location);
	void parseAllowedMethods(LocationConfig& location);
	void parseRedirect(LocationConfig& location);
	void parseAlias(LocationConfig& location);
	void parseAutoindex(LocationConfig& location);
	void parseIndex(LocationConfig& location);

	// Keys
	static const std::vector<std::string> validServerKeys;
	static const std::vector<std::string> validLocationKeys;
	static const std::vector<int> validErrorCodes;
	static const std::vector<int> validRedirectCodes;
	bool isValidServerKey(std::string key);
	bool isValidLocationKey(std::string key);
	bool isValidErrorCode(int code);
	bool isValidRedirectCode(int code);
};
