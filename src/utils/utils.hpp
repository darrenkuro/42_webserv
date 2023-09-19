#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <climits>
#include <fstream>
#include <sstream>

struct SocketAddress
{
	in_addr_t host;
	int port;

	bool operator<(const SocketAddress& other) const;
};

int createIPv4Socket();
sockaddr_in createAddress(SocketAddress address);
std::string getFileContent(std::string path);
std::string toString(int value);
std::string toIPString(in_addr_t ip);
in_addr_t toIPv4(std::string str);
int toInt(std::string str);
bool isAllDigit(std::string str);

std::string fullPath(std::string root, std::string path);
std::string getExtension(std::string path);
