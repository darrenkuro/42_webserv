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

//#include <sys/time.h> // gettimeofday
#include "infrastructure.hpp"

string getFileContent(string path);
string toString(int value);
string toIPString(in_addr_t ip);
in_addr_t toIPv4(string str);
int toInt(string str);
bool isAllDigit(string str);

string fullPath(string root, string path);
string getExtension(string path);

Address getAddressFromFd(int fd);

time_t getCurrentTime(void);
