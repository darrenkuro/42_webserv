#include <iostream>
#include <ctime>
#include <unistd.h>
#include <errno.h>
#include <cstring>

using namespace std;
using std::string;

std::string getExtension(std::string path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos && dotPos != 0) {
		return path.substr(dotPos);
	}
	return "";
}

string getScriptName(const string& uri)
{
	// if (uri.find("/cgi-bin/") == string::npos) {
	// 	//throw runtime_error("no " + "/cgi-bin/" + " in path");
	// }

	// size_t i = uri.find_first_of("/", string("/cgi-bin/").size());
	// //cout << i << endl;
	return uri.substr(uri.find_first_of("?") + 1);
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

int main(int ac, char** av, char** env) {
	char* argv[3] = {"/usr/bin/python3", "../public/cgi-bin/example.py", NULL};
	if (execve(argv[0], argv, env) == -1) {
		write(1, "c", 1);
		std::cerr << "execve failed with error: " << strerror(errno) << std::endl;
	}
	sleep(1);
	return 0;
}
