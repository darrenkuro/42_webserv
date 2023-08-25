#include "webserv.hpp"

int main(int ac, char **av) {
	(void) av;
	if (ac > 2) {
		std::cerr << ERR_ARG << std::endl;
		return 1;
	}
	
	std::cout << "TEST" << std::endl;
	return 0;
}
