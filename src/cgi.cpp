#include "cgi.hpp"
#include <cstring>

string getScriptName(const string& uri)
{
	if (uri.find(CGI_BIN) != 0) {
		throw runtime_error("no " + string(CGI_BIN) + " at the start");
	}

	size_t end = uri.find_first_of("/", string(CGI_BIN).size());
	return uri.substr(0, end);
}

string getQueryString(const string& uri)
{
	// Consider URL encoding?
	size_t sepPos = uri.find_first_of("?");
	return sepPos == string::npos ? "" : uri.substr(sepPos + 1);
}

string translatePath(const string& uri, const Server& server)
{
	// translate onto path on the filesystem
	// you need to get the related path correct? Although that seems more like a chdir after fork
	(void)uri;
	(void)server;
	return "";
}

StringMap getCgiEnv(HttpRequest& req, const Client& client, const Server& server)
{
	StringMap metaVars;

	metaVars["CONTENT_LENGTH"] = req.header["Content-Length"];
	metaVars["CONTENT_TYPE"] = req.header["Content-Type"];
	metaVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	metaVars["PATH_INFO"] = req.uri;
	metaVars["PATH_TRANSLATED"] = translatePath(req.uri, server);
	metaVars["QUERY_STRING"] = getQueryString(req.uri);
	metaVars["REMOTE_ADDR"] = ""; // MUST, need getpeername??
	metaVars["REQUEST_METHOD"] = req.method;
	metaVars["SCRIPT_NAME"] = getScriptName(req.uri);
	metaVars["SERVER_NAME"] = server.getName();
	metaVars["SERVER_PORT"] = toString(server.getAddress().port);
	metaVars["SERVER_PROTOCOL"] = HTTP_VERSION;
	metaVars["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
	(void) client;

	return metaVars;
}

char** getEnvPointer(const StringMap& envMap)
{
	char** envPointers = new char*[envMap.size() + 1];

	size_t i = 0;
	for (StringMap::const_iterator it = envMap.begin(); it != envMap.end(); it++) {
		string envString = it->first + "=" + it->second;
		envPointers[i++] = strdup(envString.c_str());
	}
	envPointers[envMap.size()] = NULL;
	return envPointers;
}

void freeEnvPointer(char** envPointer)
{
    if (!envPointer) return;

    for (char** i = envPointer; *i != NULL; ++i) {
        free(*i);
    }
    delete[] envPointer;
}
