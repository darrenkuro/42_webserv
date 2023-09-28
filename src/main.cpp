#include "Webserver.hpp"
#include "utils.hpp"
#include "log.hpp"
#include <signal.h>

#define DEFAULT_CONF "config/default.conf"

bool g_running = true;

void signalHandler(int signum) {
	if (signum == SIGINT) {
		g_running = false;
		std::cout << std::endl;
	}
}

int main(int argc, char **argv)
{
	signal(SIGINT, signalHandler);

	std::string configPath;

	if (argc == 1) {
		configPath = DEFAULT_CONF;
	}
	else if (argc == 2) {
		configPath = argv[1];
	}
	else {
		std::cerr << "[Error] Invalid argument count!" << std::endl;
		return 0;

	}
	Webserver webserver(configPath);
	webserver.start();

	return 0;
}
