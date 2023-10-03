#pragma once

#include "infrastructure.hpp"

ostream& displayTimestamp(ostream& os);
ostream& align(ostream& os);
void displayLogLevel(int level);
void log(int level, const char* format, ...);
void log(ServerConfig config);
void log(HttpRequest req, int clientID);
void log(HttpResponse res, int clientID);

ostream& operator<<(ostream& os, const HttpRequest req);
ostream& operator<<(ostream& os, const HttpResponse res);
ostream& operator<<(ostream& os, const Address address);
ostream& operator<<(ostream& os, const LocationConfig location);
ostream& operator<<(ostream& os, const ServerConfig config);
