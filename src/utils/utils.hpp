#pragma once

#include "common.hpp"

/* General Utils */
bool isAllDigit(const string& str);
string toString(int value);
string toIpString(in_addr_t ip);
string getFileContent(const string& path);
string getFileExtension(const string& path);
string fullPath(string root, string path);
int toInt(const string& str);
in_addr_t toIpNum(string str);
