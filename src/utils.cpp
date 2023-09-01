#include "utils.hpp"

int createIPv4Socket()
{
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		throw std::runtime_error("[Error] Create socket failed!");
	}
	return sockfd;
}

sockaddr_in createAddress(SocketAddress address)
{
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(address.port);
	server_addr.sin_addr.s_addr = address.host;
	//inet_pton(AF_INET, "192.200.199.140", &server_addr.sin_addr);
	return server_addr;
}

std::string getFileContent(std::string path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + path);
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

std::string toString(int value) {
	std::stringstream ss;
	ss << value;

	// Unlikely to happen, bad alloc only
	if (ss.fail()) {
		return "";
	}

	return ss.str();
}

int toInt(std::string str) {
	std::istringstream iss(str);
	int result;
	iss >> result;
	if (result == 0 && str != "0" && str != "+0" && str != "-0")
		throw std::runtime_error("convert " + str + " toInt fails");
	if (result == INT_MAX && str != "2147483647" && str != "+2147483647")
		throw std::runtime_error("convert " + str + " toInt fails");
	if (result == INT_MIN && str != "-2147483648" && str != "-2147483648")
		throw std::runtime_error("convert " + str + " toInt fails");
	return result;
}
