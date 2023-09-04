#include <iostream>

std::string getExtension(std::string path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos && dotPos != 0) {
		return path.substr(dotPos);
	}
	return "";
}

int main(int ac, char **av) {
	if (ac != 2)
		return 1;
	std::cout << getExtension(av[1]) << std::endl;
	return 0;
}
