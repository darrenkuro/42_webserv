#pragma once

#include <iostream>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ConfigParser.hpp"
#include <ctime>
#include <cstdarg>
#include <stdio.h>

namespace{
using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::map;
using std::vector;
}
#define RESET	"\033[0m"
#define CYAN	"\033[36m"
#define YELLOW	"\033[33m"
#define RED		"\033[31m"
#define ORANGE	"\033[38;5;214m"
#define DORANGE "\033[37;5;166m"
#define PADDING	21
#define DEBUG	4
#define INFO	3
#define WARNING 2
#define ERROR	1

// LOG display level configuration
#define LOG_DISPLAY_LEVEL DEBUG

/* --------------------------------------------------------------------------------------------- */
inline ostream& displayTimestamp(ostream& os)
{
	std::time_t result = std::time(NULL);
	string timestamp(std::ctime(&result));
	os << CYAN << "[" + timestamp.substr(11, 8) + "]" << RESET;
	return os;
}

inline ostream& align(ostream& os)
{
	string padding(PADDING, ' ');
	os << padding;
	return os;
}

inline void displayLogLevel(int level)
{
	switch (level)
	{
	case DEBUG:
		cout << YELLOW << "[DEBUG]" << RESET << "   ";
		break;

	case INFO:
		cout << CYAN << "[INFO]" << RESET << "    ";
		break;

	case WARNING:
		cout << RED << "[WARNING]" << RESET << " ";
		break;

	case ERROR:
		cout << RED << "[ERROR]" << RESET << "   ";
		break;
	}
}

inline void log(int level, const char* format, ...)
{
	if (level > LOG_DISPLAY_LEVEL) {
		return;
	}
	cout << displayTimestamp << " ";
	displayLogLevel(level);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	cout << endl;
}

/* --------------------------------------------------------------------------------------------- */
inline ostream& operator<<(ostream& os, HttpRequest req)
{
	os << align;
	(void)req;
	os << "Method[" << req.method << "] ";
	os << "Uri[" << req.uri << "] ";
	os << "Version[" << req.version << "] ";
	os << "Host[" << req.header.find("Host")->second << "] ";
	return os;
}

inline ostream& operator<<(ostream& os, HttpResponse res)
{
	os << align;
	os << "Status[" << res.statusCode << "]";
	return os;
}

inline ostream &operator<<(ostream &os, const Address address)
{
	// os << "Address[" << toIPString(address.host) << ":";
	os << address.port << "]";
	return os;
}

inline ostream &operator<<(ostream &os, const LocationConfig location)
{
	os << "URI[" << location.uri << "] ";
	os << "Alias[" << location.alias << "] ";
	os << "Autoindex[" << location.autoindex << "] ";
	os << "Redirect[" << location.redirect.first << ":";
	os << location.redirect.second << "] AllowedMethods[";

	vector<string>::const_iterator it;
	for (it = location.allowedMethods.begin(); it != location.allowedMethods.end(); it++) {
		os << " " << *it;
	}
	os << " ] Index[";
	for (it = location.index.begin(); it != location.index.end(); it++) {
			os << " " << *it;
	}
	os << " ]";
	return os;
}

inline ostream &operator<<(ostream &os, const ServerConfig config)
{
	os << "ServerName[" << config.serverName << "] ";
	os << "Root[" << config.root << "] ";
	os << "MaxBodySize[" << config.clientMaxBodySize << "] ";
	os << config.address << endl;

	os << align, os << "Locations: " << endl;
	vector<LocationConfig>::const_iterator it2;
	for (it2 = config.locations.begin(); it2 != config.locations.end(); it2++) {
		os << align;
		os << ">> " << *it2 << endl;
	}

	os << align, os << "ErrorPages: ";
	map<int, string>::const_iterator it;
	int i = 0;
	for (it = config.errorPages.begin(); it != config.errorPages.end(); it++) {
		if (i++ % 3 == 0)
			os << endl, os << align, os << ">> ";
		os << it->first << "[" << it->second << "] ";
	}
	os << endl;
	return os;
}

/* --------------------------------------------------------------------------------------------- */
inline void logHttp(HttpRequest req, int clientID)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << ORANGE;
	cout << "HTTP Req>> Client[ID " << clientID << "]" << endl;
	cout << req << RESET << endl;
}

inline void logHttp(HttpResponse res, int clientID)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << ORANGE;
	cout << "HTTP <<Res Client[ID " << clientID << "]" << endl;
	cout << res << RESET << endl;
}

inline void logServerConfig(ServerConfig config)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << "ServerConfig: ";
	cout << config;
}
