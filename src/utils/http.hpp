#pragma once

#include "common.hpp"

/* HTTP Request Parser */
HttpRequest parseHttpRequest(string& content);

/* HTTP Request Helpers */
void parseRequestPart(const string& sep, string& field, string& content);
void parseRequestHeader(StringMap& header, string& content);

/* HTTP Reponse Builders */
HttpResponse createHttpResponse(int code, const string& path);
HttpResponse createHttpResponse(const string& str);
HttpResponse createAutoindex(const string& path);

/* HTTP Response Utils */
string getStatusText(int code);
string getMimeType(string path);
string getDateString(void);
string toString(HttpResponse res);
