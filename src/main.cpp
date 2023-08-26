#include "Webserv.hpp"

#include <wait.h>

#define ERR_ARG "[Error] Wrong number of arguments!"

int main(int argc, char **argv)
{
	if (argc > 2) {
		std::cerr << ERR_ARG << std::endl;
		return 1;
	}
	(void) argv;
	
	WebservConfig config1;
	config1.serverName = "Server1";
	config1.port = 4243;
	WebservConfig config2;
	config2.serverName = "Server2";
	config2.port = 4244;

	Webserv webserv1(config1);
	Webserv webserv2(config2);

	webserv1.launch();
	webserv2.launch();

	while (true) {}
	sleep(10);
	webserv1.stop();
	sleep(5);
	webserv2.stop();

	wait(NULL);
	
	std::cout << "TEST" << std::endl;
	return 0;
}
