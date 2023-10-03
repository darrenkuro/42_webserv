#pragma once

#include <vector>
#include <map>
#include <string>
#include "global.hpp"
#include "utils.hpp"

class ConfigParser
{
public:
	vector<ServerConfig> parse(const string& filename);

	static bool isValidErrorCode(int code);
	static bool isValidRedirectCode(int code);

private:
	vector<ServerConfig> m_configs;
	deque<string> m_tokens;

	// Logic
	void lex(const string& content, const string& whitespace, const string& symbol);

	string accept(void);
	void consume(const string& token);

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
	static const vector<string> validServerKeys;
	static const vector<string> validLocationKeys;
	static const vector<int> validErrorCodes;
	static const vector<int> validRedirectCodes;
	bool isValidServerKey(string key);
	bool isValidLocationKey(string key);

	// Default Settings
	ServerConfig createServer(void);
	LocationConfig createLocation(void);
	void addDefaultErrorPages(ServerConfig& server);
	void addDefaultLocation(ServerConfig& server);
};
