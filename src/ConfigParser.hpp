#pragma once

#include "common.hpp"

class ConfigParser
{
public:
	/* Main Parser */
	vector<ServerConfig> parse(const string& filename);

	/* Validation */
	static bool isValidErrorCode(int code);
	static bool isValidRedirectCode(int code);
	static bool isValidServerKey(string key);
	static bool isValidLocationKey(string key);

private:
	/* Logic */
	void lex(const string& content, const string& whitespace, const string& symbol);

	string accept(void);
	void consume(const string& token);

	/* Context Parsers */
	void parseServer(void);
	void parseLocation(ServerConfig& server);

	/* Server Field Parsers */
	void parseRoot(ServerConfig& server);
	void parseServerName(ServerConfig& server);
	void parseAddress(ServerConfig& server);
	void parseClientMaxBodySize(ServerConfig& server);
	void parseErrorPage(ServerConfig& server);

	/* Location Field Parsers */
	void parseUri(LocationConfig& location);
	void parseAutoindex(LocationConfig& location);
	void parseAlias(LocationConfig& location);
	void parseAllowedMethods(LocationConfig& location);
	void parseIndex(LocationConfig& location);
	void parseRedirect(LocationConfig& location);

	/* Keys */
	static const StringVec validServerKeys;
	static const StringVec validLocationKeys;
	static const vector<int> validErrorCodes;
	static const vector<int> validRedirectCodes;

	/* Default Settings */
	ServerConfig createServer(void);
	LocationConfig createLocation(void);
	void addDefaultErrorPages(ServerConfig& server);
	void addDefaultLocation(ServerConfig& server);

	/* Member Data */
	vector<ServerConfig> m_configs;
	deque<string> m_tokens;
};
