#pragma once

#include "infrastructure.hpp"
#include "Server.hpp"
#include "Client.hpp"

string getScriptName(const string& uri);
string getQueryString(const string& uri);
StringMap getCgiEnv(HttpRequest& req, const Client& client, const Server& server);
char** getEnvPointer(const StringMap& envMap);
//void freeEnvPointer(char** envPointer);

string executeCgi(const StringMap& envMap, int& code);
HttpResponse processCgiRequest(HttpRequest req, const Client& client, const Server& server);
