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

// Base case for the recursive template function
template <typename T>
void customPrint(const T& arg) {
    std::cout << arg;
}

// Recursive template function to print arguments
template <typename T, typename... Args>
void customPrint(const T& arg, const Args&... args) {
    std::cout << arg; // Output the current argument
    customPrint(args...); // Recur for the remaining arguments
}

std::ostream& appendFunction(std::ostream& os, int arg1, double arg2) {
    os << "Function result: " << (arg1 + arg2);
    return os;
}

int main(int ac, char **av) {
	customPrint("test", "ugh", 1, 42);
	return 0;
}
