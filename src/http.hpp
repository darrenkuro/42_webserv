#pragma once

#include "infrastructure.hpp"

HttpRequest parseHttpRequest(string content);

void parseRequestPart(const string& sep, string& field, string& content);
void parseRequestHeader(StringMap& header, string& content);

HttpResponse createBasicResponse(int code, string path);

string getDate(void);
string getStatusText(int code);
string getMimeType(string path);
string toString(HttpResponse res);
