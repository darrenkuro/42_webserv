#pragma once

#include "infrastructure.hpp"
#include "Server.hpp"
#include "Client.hpp"

string getScriptName(const string& uri);
string getQueryString(const string& uri);
string translatePath(const string& uri, const Server& server);
StringMap getCgiEnv(HttpRequest& req, const Server& server);
char** getEnvPointer(const StringMap& envMap);
void freeEnvPointer(char** envPointer);

// HttpResponse executeCgi(const StringMap& envMap, const Server& server);
string executeCgi(const StringMap& envMap, int& code);
