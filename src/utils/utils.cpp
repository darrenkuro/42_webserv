#include "utils.hpp"

int createIPv4Socket()
{
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Create socket failed!");

	return sockfd;
}

std::string getFileContent(std::string path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + path);

	std::string content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());
	return content;
}

std::string toString(int value) {
	std::stringstream ss;
	ss << value;

	// Unlikely to happen, bad alloc only
	if (ss.fail())
		return "";

	return ss.str();
}

int toInt(std::string str) {
	std::istringstream iss(str);
	int result;
	char remainingChar;

	if (!(iss >> result) || (iss.get(remainingChar)))
		throw std::runtime_error("convert " + str + " toInt fails");
	if (result == 0 && str != "0" && str != "+0" && str != "-0")
		throw std::runtime_error("convert " + str + " toInt fails");
	if (result == INT_MAX && str != "2147483647" && str != "+2147483647")
		throw std::runtime_error("convert " + str + " toInt fails");
	if (result == INT_MIN && str != "-2147483648")
		throw std::runtime_error("convert " + str + " toInt fails");

	return result;
}


sockaddr_in createAddress(SocketAddress address)
{
	sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(address.port);
	server_addr.sin_addr.s_addr = address.host;

	return server_addr;
}

bool SocketAddress::operator<(const SocketAddress& other) const
{
	if (host < other.host)
		return true;
	if (host > other.host)
		return false;
	return port < other.port;
}

std::string toIPString(in_addr_t ip)
{
	std::ostringstream oss;
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&ip);

	oss << static_cast<int>(bytes[0]) << '.'
		<< static_cast<int>(bytes[1]) << '.'
		<< static_cast<int>(bytes[2]) << '.'
		<< static_cast<int>(bytes[3]);

	return oss.str();
}

in_addr_t toIPv4(std::string str)
{
	in_addr_t result = 0;

	for (int i = 0; i < 4; i++) {
		if (i < 3 && str.find('.') == std::string::npos)
			throw std::runtime_error("failed to convert " + str);

		std::string token = i < 3 ? str.substr(0, str.find('.')) : str;
		int value = toInt(token);
		if (!isAllDigit(token) || value < 0 || value > 255)
			throw std::runtime_error("failed to convert " + str);

		result = (result << 8) | value;

		if (i < 3)
			str.erase(str.begin(), str.begin() + str.find('.') + 1);
	}
	return htonl(result);
}

bool isAllDigit(std::string str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

std::string fullPath(std::string root, std::string path)
{
	if (root.empty() || path.empty())
		return "";

	// Remove '/' from root and path
	root = root[root.size() - 1] == '/' ? root.substr(0, root.size() - 1) : root;
	path = path[0] == '/' ? path.substr(1) : path;

	return root + "/" + path;
}

std::string getExtension(std::string path)
{
	size_t pos = path.find_last_of('.');

	if (pos != std::string::npos && pos != 0)
		return path.substr(pos);

	return "";
}
