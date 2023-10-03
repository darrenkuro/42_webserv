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

#include <sys/time.h> // gettimeofday
#include "global.hpp"

string getFileContent(string path);
string toString(int value);
string toIPString(in_addr_t ip);
in_addr_t toIPv4(string str);
int toInt(string str);
bool isAllDigit(string str);

string fullPath(string root, string path);
string getExtension(string path);
int createTcpListenSocket(Address addr);
Address getAddressFromFd(int fd);
pollfd buildPollFd(int fd, short events);


timeval getTime(void);

string getDate(void);
string getStatusText(int code);
string getMimeType(string path);
string toString(HttpResponse res);

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
