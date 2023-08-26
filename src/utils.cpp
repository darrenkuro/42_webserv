#include "utils.hpp"

int createIPv4Socket()
{
	int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("[Error] Create socket failed!");
    }
	return sockfd;
}

struct sockaddr_in createAddress(int port)
{
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	return server_addr;
}