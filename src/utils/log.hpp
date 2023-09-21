#pragma once

#include <iostream>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ConfigParser.hpp"
#include <ctime>
#include <cstdarg>
#include <stdio.h>

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

/* -------------------------------------------------------------------------- *
 * Utilities.
 * -------------------------------------------------------------------------- */
inline std::ostream& displayTimestamp(std::ostream& os)
{
	std::time_t result = std::time(NULL);
	std::string timestamp(std::ctime(&result));
	os << CYAN << "[" + timestamp.substr(11, 8) + "]" << RESET;
	return os;
}

/* -------------------------------------------------------------------------- */
inline std::ostream& align(std::ostream& os)
{
	std::string padding(PADDING, ' ');
	os << padding;
	return os;
}

/* -------------------------------------------------------------------------- */
inline void displayLogLevel(int level)
{
	switch (level)
	{
	case DEBUG:
		std::cout << YELLOW << "[DEBUG]" << RESET << "   ";
		break;

	case INFO:
		std::cout << CYAN << "[INFO]" << RESET << "    ";
		break;

	case WARNING:
		std::cout << RED << "[WARNING]" << RESET << " ";
		break;

	case ERROR:
		std::cout << RED << "[ERROR]" << RESET << "   ";
		break;
	}
}

/* -------------------------------------------------------------------------- */
inline void log(int level, const char* format, ...)
{
	if (level > LOG_DISPLAY_LEVEL) {
		return;
	}
	std::cout << displayTimestamp << " ";
	displayLogLevel(level);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	std::cout << std::endl;
}

/* -------------------------------------------------------------------------- *
 * Stream insertion operator overloads for custom structs and classes.
 * -------------------------------------------------------------------------- */
inline std::ostream& operator<<(std::ostream& os, HttpRequest req)
{
	os << align;
	os << "Method[" << req.method << "] ";
	os << "Uri[" << req.uri << "] ";
	os << "Version[" << req.version << "] ";
	os << "Host[" << req.headers.find("Host")->second << "] ";
	os << "Content-Length[" << req.headers.find("Content-Length")->second << "]";
	return os;
}

/* -------------------------------------------------------------------------- */
inline std::ostream& operator<<(std::ostream& os, HttpResponse res)
{
	os << align;
	os << "Status[" << res.statusCode << "]";
	return os;
}

/* -------------------------------------------------------------------------- */
inline std::ostream &operator<<(std::ostream &os, const SocketAddress address)
{
	os << "Address[" << toIPString(address.host) << ":";
	os << address.port << "]";
	return os;
}

/* -------------------------------------------------------------------------- */
inline std::ostream &operator<<(std::ostream &os, const LocationConfig location)
{
	os << "URI[" << location.uri << "] ";
	os << "Alias[" << location.alias << "] ";
	os << "Autoindex[" << location.autoindex << "] ";
	os << "Redirect[" << location.redirect.first << ":";
	os << location.redirect.second << "] AllowedMethods[";

	std::vector<std::string>::const_iterator it;
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

/* -------------------------------------------------------------------------- */
inline std::ostream &operator<<(std::ostream &os, const ServerConfig config)
{
	os << "ServerName[" << config.serverName << "] ";
	os << "Root[" << config.root << "] ";
	os << "MaxBodySize[" << config.clientMaxBodySize << "] ";
	os << config.address << std::endl;

	os << align, os << "Locations: " << std::endl;
	std::vector<LocationConfig>::const_iterator it2;
	for (it2 = config.locations.begin(); it2 != config.locations.end(); it2++) {
		os << align;
		os << ">> " << *it2 << std::endl;
	}

	os << align, os << "ErrorPages: ";
	std::map<int, std::string>::const_iterator it;
	int i = 0;
	for (it = config.errorPages.begin(); it != config.errorPages.end(); it++) {
		if (i++ % 3 == 0)
			os << std::endl, os << align, os << ">> ";
		os << it->first << "[" << it->second << "] ";
	}
	os << std::endl;
	return os;
}

/* -------------------------------------------------------------------------- *
 * Log with information and color for structures and classes.
 * -------------------------------------------------------------------------- */
inline void logHttp(HttpRequest req, int clientID)
{
	std::cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	std::cout << ORANGE;
	std::cout << "HTTP Req>> Client[ID " << clientID << "]" << std::endl;
	std::cout << req << RESET << std::endl;
}

/* -------------------------------------------------------------------------- */
inline void logHttp(HttpResponse res, int clientID)
{
	std::cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	std::cout << ORANGE;
	std::cout << "HTTP <<Res Client[ID " << clientID << "]" << std::endl;
	std::cout << res << RESET << std::endl;
}

/* -------------------------------------------------------------------------- */
inline void logServerConfig(ServerConfig config)
{
	std::cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	std::cout << "ServerConfig: ";
	std::cout << config;
}
