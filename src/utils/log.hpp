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

inline std::ostream& displayTimestamp(std::ostream& os)
{
	std::time_t result = std::time(NULL);
	std::string timestamp(std::ctime(&result));
	os << CYAN << "[" + timestamp.substr(11, 8) + "]" << RESET;
	return os;
}

inline std::ostream& align(std::ostream& os)
{
	std::string padding(PADDING, ' ');
	os << padding;
	return os;
}

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

inline void log(int level, const char* format, ...)
{
	if (level > LOG_DISPLAY_LEVEL)
		return;
	std::cout << displayTimestamp << " ";
	displayLogLevel(level);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	std::cout << std::endl;
}

// Custom class / struct << operator overloading
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

inline std::ostream& operator<<(std::ostream& os, HttpResponse res)
{
	os << align;
	os << "Status[" << res.statusCode << "]";
	return os;
}

inline void logServerConfig(ServerConfig config)
{
	std::cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	std::cout << ORANGE;
	std::cout << "======> WebservConfig" << std::endl;
	std::cout << "listen: " << std::endl;
	std::cout << "\thost: " << config.address.host << std::endl;
	std::cout << "\tport: " << config.address.port << std::endl;
	std::cout << "server_name: " << config.serverName << std::endl;
	std::cout << "location:" << std::endl;
	for(std::vector<LocationConfig>::const_iterator it = config.locations.begin(); it != config.locations.end(); it++) {
		std::cout << "\tautoindex: " << std::boolalpha << it->autoindex << std::endl;
	}
	std::cout << RESET << std::endl;
}

// ----------------------------------------------------------------------------

inline void logHttp(HttpRequest req, int clientID)
{
	std::cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	std::cout << ORANGE;
	std::cout << "HTTP Req>> Client[ID " << clientID << "]" << std::endl;
	std::cout << req << RESET << std::endl;
}

inline void logHttp(HttpResponse res, int clientID)
{
	std::cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	std::cout << ORANGE;
	std::cout << "HTTP <<Res Client[ID " << clientID << "]" << std::endl;
	std::cout << res << RESET << std::endl;
}
