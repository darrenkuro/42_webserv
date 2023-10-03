#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>
#include <climits>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <poll.h>
#include <ctime>
#include <sstream>

#include "global.hpp"

std::string getFileContent(std::string path);
std::string toString(int value);
std::string toIPString(in_addr_t ip);
in_addr_t toIPv4(std::string str);
int toInt(std::string str);
bool isAllDigit(std::string str);

std::string fullPath(std::string root, std::string path);
std::string getExtension(std::string path);
int createTcpListenSocket(Address addr);
Address getAddressFromFd(int fd);
pollfd buildPollFd(int fd, short events);

std::string getDate(void);
std::string getStatusText(int code);
std::string getMimeType(std::string path);
std::string toString(HttpResponse res);

ostream& displayTimestamp(ostream& os);
ostream& align(ostream& os);
void displayLogLevel(int level);
void log(int level, const char* format, ...);
void log(ServerConfig config);
void log(HttpRequest req, int clientID);
void log(HttpResponse res, int clientID);

ostream& operator<<(ostream& os, HttpRequest req);
ostream& operator<<(ostream& os, HttpResponse res);
ostream &operator<<(ostream &os, const Address address);
ostream &operator<<(ostream &os, const LocationConfig location);
ostream &operator<<(ostream &os, const ServerConfig config);

HttpResponse createBasicResponse(int code, string path);
