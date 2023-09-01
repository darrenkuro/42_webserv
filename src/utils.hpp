#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>
#include "ConfigParser.hpp"

struct SocketAddress;

int createIPv4Socket();
sockaddr_in createAddress(SocketAddress address);
std::string getFileContent(std::string path);
std::string toString(int value);
int toInt(std::string str);
