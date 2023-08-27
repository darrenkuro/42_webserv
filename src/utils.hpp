#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

int createIPv4Socket();
sockaddr_in createAddress(int port);
std::string getFileContent(std::string path);
std::string intToString(int value);
