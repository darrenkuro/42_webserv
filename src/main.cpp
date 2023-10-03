#include "Webserver.hpp"
#include "utils.hpp"
#include "infrastructure.hpp"
#include <signal.h>

bool g_running = true;

void signalHandler(int signum) {
	if (signum == SIGINT) {
		g_running = false;
		std::cout << std::endl;
	}
}

int main(int argc, char **argv)
{
	if (argc > 2) {
		cerr << "[Error] Invalid argument count!" << endl;
		return 1;
	}

	signal(SIGINT, signalHandler);
	string configPath = argc == 1 ? DEFAULT_CONF : argv[1];
	Webserver webserver(configPath);
	webserver.start();

	return 0;
}
