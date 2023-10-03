#pragma once

#include "infrastructure.hpp"
#include "Server.hpp"
#include "Client.hpp"

string getScriptName(const string& uri);
string getQueryString(const string& uri);
string translatePath(const string& uri, const Server& server);
StringMap getCgiEnv(HttpRequest& req, const Client& client, const Server& server);
char** getEnvPointer(const StringMap& envMap);
void freeEnvPointer(char** envPointer);

void executeCgi();
