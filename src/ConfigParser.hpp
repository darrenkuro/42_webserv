#pragma once

#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <netinet/in.h>
#include "utils.hpp"

struct LocationConfig
{
	bool autoindex;
	std::string uri;
	std::string redirect;
	std::string alias;
	std::vector<std::string> index;
	std::vector<std::string> allowedMethods;
};

struct SocketAddress
{
	in_addr_t host;
	int port;
};

struct ServerConfig
{
	std::string serverName;
	std::string root;
	SocketAddress address;
	size_t clientMaxBodySize;
	std::map<int, std::string> errorPages;
	std::vector<LocationConfig> locations;
};

#define ERR_SYNTAX	"Parser syntax error!"

class ConfigParser
{
public:
	std::vector<ServerConfig> parse(const std::string filename);

private:
	std::deque<std::string> m_tokens;

	static const std::vector<std::string> validServerKeys;
	static const std::vector<std::string> validLocationKeys;

	bool isValidServerKey(std::string key);
	bool isValidLocationKey(std::string key);
	bool isAllDigit(std::string str);

	void lex(std::string content, std::string whitespace, std::string symbol);
	void consume(const std::string token);
	std::string accept(void);

	ServerConfig defaultServer(void);
	LocationConfig defaultLocation(void);

	ServerConfig parseServer(void);
	LocationConfig parseLocation(void);
	SocketAddress parseListen(void);
	std::string parseServerName(void);
	std::pair<int, std::string> parseErrorPage(void);
	size_t parseClientMaxBodySize(void);
	std::string parseUri(void);
	std::vector<std::string> parseAllowedMethods(void);
	std::string parseRedirect(void);
	std::string parseAlias(void);
	bool parseAutoindex(void);
	std::vector<std::string> parseIndex(void);
	// std::string parseRoot(void);
};
