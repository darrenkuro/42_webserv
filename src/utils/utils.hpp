#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <climits>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <poll.h>
#include <ctime>
#include <sstream>
#include <set>

struct SocketAddress
{
	in_addr_t ip;
	int port;
};

struct Address
{
	in_addr_t ip;
	int port;

	bool operator<(const Address& rhs) const;
	std::string ipToStr();
	std::string portToStr();
};

int createIPv4Socket();
sockaddr_in createAddress(SocketAddress address);
std::string getFileContent(std::string path);
std::string toString(int value);
std::string toIPString(in_addr_t ip);
in_addr_t toIPv4(std::string str);
int toInt(std::string str);
bool validatePort(const std::string& port);
bool validateIpAddress(const std::string& ipAddress);
bool isAllDigit(std::string str);

std::string fullPath(std::string root, std::string path);
std::string getExtension(std::string path);
int createTcpListenSocket(Address addr);
Address getAddressFromFd(int fd);
pollfd buildPollFd(int fd, short events);
