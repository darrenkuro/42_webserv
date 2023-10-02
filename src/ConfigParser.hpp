#pragma once

#include <vector>
#include <deque>
#include <map>
#include <string>
#include "utils.hpp"

struct LocationConfig
{
	bool autoindex;
	std::string uri;
	std::string alias;
	std::vector<std::string> allowedMethods;
	std::vector<std::string> index;
	std::pair<int, std::string> redirect;
};

struct ServerConfig
{
	std::string root;
	Address address;
	std::vector<std::string> index;
	std::string serverName;
	ssize_t clientMaxBodySize;
	std::map<int, std::string> errorPages;
	std::vector<LocationConfig> locations;
};

class ConfigParser
{
public:
	std::vector<ServerConfig> parse(const std::string& filename);

	static bool isValidErrorCode(int code);
	static bool isValidRedirectCode(int code);

private:
	std::vector<ServerConfig> m_configs;
	std::deque<std::string> m_tokens;

	// Logic
	void lex(const std::string& content, const std::string& whitespace, const std::string& symbol);

	std::string accept(void);
	void consume(const std::string& token);

	// Context & Field Parser
	void parseServer(void);
	void parseLocation(ServerConfig& server);

	void parseRoot(ServerConfig& server);
	void parseServerName(ServerConfig& server);
	void parseAddress(ServerConfig& server);
	void parseClientMaxBodySize(ServerConfig& server);
	void parseErrorPage(ServerConfig& server);

	void parseUri(LocationConfig& location);
	void parseAutoindex(LocationConfig& location);
	void parseAlias(LocationConfig& location);
	void parseAllowedMethods(LocationConfig& location);
	void parseIndex(LocationConfig& location);
	void parseRedirect(LocationConfig& location);

	// Keys
	static const std::vector<std::string> validServerKeys;
	static const std::vector<std::string> validLocationKeys;
	static const std::vector<int> validErrorCodes;
	static const std::vector<int> validRedirectCodes;
	bool isValidServerKey(std::string key);
	bool isValidLocationKey(std::string key);

	// Default Settings
	ServerConfig createServer(void);
	LocationConfig createLocation(void);
	void addDefaultErrorPages(ServerConfig& server);
	void addDefaultLocation(ServerConfig& server);
};
