#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <climits>
#include <fstream>
#include <ctime>
#include <sstream>

struct SocketAddress
{
	in_addr_t host;
	int port;
};

int createIPv4Socket();
sockaddr_in createAddress(SocketAddress address);
std::string getFileContent(std::string path);
std::string toString(int value);
std::string toIPString(in_addr_t ip);
in_addr_t toIPv4(std::string str);
int toInt(std::string str);
bool isAllDigit(std::string str);
