#include "utils.hpp"

int createIPv4Socket()
{
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		throw std::runtime_error("[Error] Create socket failed!");
	}
	return sockfd;
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


sockaddr_in createAddress(SocketAddress address)
{
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(address.port);
	server_addr.sin_addr.s_addr = address.ip;
	//inet_pton(AF_INET, "192.200.199.140", &server_addr.sin_addr);
	return server_addr;
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

bool validatePort(const std::string& port)
{
    // Validate that the port is a positive integer
    try {
        int num = atoi(port.c_str());
        return num > 0 && num <= 65535; // Valid port range
    } catch (const std::invalid_argument& e) {
        return false; // Port is not an integer
    }
}

bool validateIpAddress(const std::string& ipAddress)
{
    // Split the IP address by "." and validate each part
    std::istringstream ss(ipAddress);
    std::string part;
    int numParts = 0;

    while (std::getline(ss, part, '.')) {
        int num =atoi(part.c_str());
        if (num < 0 || num > 255) {
            return false; // Invalid IP part
        }
        numParts++;
    }

    return numParts == 4; // IP should have exactly 4 parts
}

in_addr_t toIPv4(std::string str)
{
	in_addr_t result = 0;
	for (int i = 0; i < 4; i++) {
		if (i < 3 && str.find('.') == std::string::npos) {
			throw std::runtime_error("failed to convert " + str);
		}

		std::string token = i < 3 ? str.substr(0, str.find('.')) : str;
		int value = toInt(token);
		if (!isAllDigit(token) || value < 0 || value > 255)
			throw std::runtime_error("failed to convert " + str);
		result = (result << 8) | value;

		if (i < 3) {
			str.erase(str.begin(), str.begin() + str.find('.') + 1);
		}
	}
	return htonl(result);
}

bool isAllDigit(std::string str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i])) {
			return false;
		}
	}
	return true;
}

std::string fullPath(std::string root, std::string path)
{
	root = root[root.size() - 1] == '/' ? root.substr(0, root.size() - 1) : root;
	path = path[0] == '/' ? path.substr(1) : path;
	return root + "/" + path;
}

std::string getExtension(std::string path)
{
	size_t pos = path.find_last_of('.');
	if (pos != std::string::npos && pos != 0) {
		return path.substr(pos);
	}
	return "";
}

int createTcpListenSocket(Address addr)
{
	int fd;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("socket() failed" + std::string(strerror(errno)));

	int sockopt = 1;
  	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &sockopt, sizeof (int)) == -1)
		throw std::runtime_error("setsockopt() failed: " + std::string(strerror(errno)));

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(addr.port);
	serverAddr.sin_addr.s_addr = addr.ip;

	if (bind(fd, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		close(fd);
		throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
	}

	if (listen(fd, 10))
		throw std::runtime_error("listen() failed" + std::string(strerror(errno)));

	return fd;
}

bool Address::operator<(const Address& rhs)
{
    if (ip < rhs.ip)
        return true;
    if (ip > rhs.ip)
        return false;
    return port < rhs.port;
}

std::set<Address> getUniqueAddresses(std::vector<Server> servers)
{
	std::set<Address> uniques;
	for (size_t i = 0; i < servers.size(); i++) {
		if (servers[i].getAddress().ip == 0) {
			uniques.insert(servers[i].getAddress());			
		}
	}

	for (size_t i = 0; i < servers.size(); i++) {
		Address addr = servers[i].getAddress();
		if (addr.ip != 0) {
			std::set<Address>::iterator it;
			for (it = uniques.begin(); it != uniques.end(); it++) {
				if (it->ip == 0 && it->port == addr.port)
					break;
			}
			if (it == uniques.end())
				uniques.insert(addr);
		}
	}
	return uniques;
}


Address getAddressFromFd(int fd)
{
	struct sockaddr_in serverAddress;
    socklen_t addrLen = sizeof(serverAddress);
	getsockname(fd, (struct sockaddr*)&serverAddress, &addrLen);

	Address addr;
	addr.ip = serverAddress.sin_addr.s_addr;
	addr.port = serverAddress.sin_port;
	return addr;
}


pollfd buildPollFd(int fd, short events)
{
	pollfd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	return pfd;
}
