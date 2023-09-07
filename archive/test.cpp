#include <iostream>
#include <ctime>

std::string getExtension(std::string path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos && dotPos != 0) {
		return path.substr(dotPos);
	}
	return "";
}

int main(int ac, char **av) {
    std::time_t t = std::time(NULL);
    std::tm* timePtr = std::gmtime(&t);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timePtr);

    std::cout << "Current date and time: " << buffer << std::endl;

    return 0;
}
