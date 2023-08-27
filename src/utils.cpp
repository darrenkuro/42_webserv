#include "utils.hpp"

int createIPv4Socket()
{
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		throw std::runtime_error("[Error] Create socket failed!");
	}
	return sockfd;
}

sockaddr_in createAddress(int port)
{
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	return server_addr;
}

std::string getFileContent(std::string path)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + path);
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

std::string intToString(int value) {
	std::stringstream ss;
	ss << value;

	// Unlikely to happen, bad alloc only
	if (ss.fail()) {
		return "";
	}

	return ss.str();
}
