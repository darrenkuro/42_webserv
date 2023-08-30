#pragma once

#include <iostream>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <ctime>
#include <cstdarg>
#include <stdio.h>

#define RESET   "\033[0m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define ORANGE	"\033[38;5;214m"
#define DEBUG	4
#define INFO	3
#define WARNING 2
#define ERROR	1

// LOG display level configuration
#define LOG_DISPLAY_LEVEL INFO

inline void displayTimestamp()
{
	std::time_t result = std::time(NULL);
	std::string timestamp(std::ctime(&result));
	std::cout << CYAN << "[" + timestamp.substr(11, 8) + "]" << RESET;
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
    if (level > LOG_DISPLAY_LEVEL) return;
    displayTimestamp();
    std::cout << " ";
    displayLogLevel(level);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    std::cout << std::endl;
}

inline void logHttp(HttpRequest request, int clientID)
{
	displayTimestamp();
    std::cout << " ";
    displayLogLevel(INFO);

	std::cout << ORANGE;
	std::cout << "HTTP >> Client[ID " << clientID << "]  |  Method[" << request.method << "]";
	std::cout << RESET << std::endl;
}

inline void logHttp(HttpResponse response, int clientID)
{
	(void)response;
	displayTimestamp();
    std::cout << " ";
    displayLogLevel(INFO);

	std::cout << ORANGE;
	std::cout << "HTTP << Client[ID " << clientID << "]  |  STATUS[OK]";
	std::cout << RESET << std::endl;
}
