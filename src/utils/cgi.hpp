#pragma once

#include "common.hpp"
#include "Server.hpp"
#include "Client.hpp"

/* Main Handler */
HttpResponse processCgiRequest(HttpRequest req, const Client& client, const Server& server);
string executeCgi(const StringMap& envMap, const string& reqBody);

/* Environmental variable Getters */
StringMap getCgiEnv(HttpRequest& req, const Client& client, const Server& server);
string getScriptName(const string& uri);
string getQueryString(const string& uri);
char** getEnvPointer(const StringMap& envMap);
char** getArgvPointer(const StringMap& envMap);
