#include <signal.h>

#include "Webserver.hpp"
#include "log.hpp"

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
	Webserver webserver(argc == 1 ? DEFAULT_CONF : argv[1]);
	webserver.start();

	return 0;
}
