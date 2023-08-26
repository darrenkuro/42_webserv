#pragma once

#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>

int createIPv4Socket();
struct sockaddr_in createAddress(int port);