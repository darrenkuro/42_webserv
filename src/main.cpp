#include "Webserver.hpp"
#include "log.hpp"
#include <signal.h>

bool g_running = true;

void signalHandler(int signum) {
	if (signum == SIGINT) {
		g_running = false;
		cout << endl;
	}
}

int main(int argc, char** argv)
{
	if (argc > 2) {
		log(ERROR, "Invalid argument count!");
		return 1;
	}

	signal(SIGINT, signalHandler);
	string configPath = argc == 1 ? DEFAULT_CONF : argv[1];
	Webserver webserver(configPath);
	webserver.start();

	return 0;
}
