#pragma once

#include "common.hpp"

/* General Utils */
bool isAllDigit(const string& str);
string toString(int value);
string toIpString(in_addr_t ip);
string getFileContent(const string& path);
string getFileExtension(const string& path);
string fullPath(string root, string path);
string getBoundary(const HttpRequest& req);
int toInt(const string& str);
in_addr_t toIpNum(string str);

/* Struct Stream Operators */
ostream& operator<<(ostream& os, const HttpRequest req);
ostream& operator<<(ostream& os, const HttpResponse res);
ostream& operator<<(ostream& os, const Address address);
ostream& operator<<(ostream& os, const LocationConfig location);
ostream& operator<<(ostream& os, const ServerConfig config);
