#pragma once

#include "common.hpp"

bool isAllDigit(const string& str);
string toString(int value);
string toIPString(in_addr_t ip);
string getFileExtension(const string& path);
string getFileContent(const string& path);
string fullPath(string root, string path);
int toInt(const string& str);
in_addr_t toIPv4(string str);
