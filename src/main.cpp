#include "Webserver.hpp"
#include "utils.hpp"

#include <signal.h>

#define ERR_ARG "[Error] Wrong number of arguments!"

#define DEFAULT_CONF "configs/default.conf"

bool g_running = true;

//-----------------------------------------------------------------------------------------


void signalHandler(int signum) {
	if (signum == SIGINT) {
		g_running = false;;
		std::cout << std::endl;
	}
}

int main(int argc, char **argv)
{
	try {
		std::vector<ServerConfig> servers;
		ConfigParser parser;
		signal(SIGINT, signalHandler);

		if (argc == 1) {
			servers = parser.parse(DEFAULT_CONF);
		}
		else if (argc == 2) {
			servers = parser.parse(argv[1]);
		}
		else {
			std::cerr << ERR_ARG << std::endl;
			return 0;
		}

		Webserver webserver(servers);
		webserver.start();

	} catch (std::exception &e) {
		log(ERROR, e.what());
	}
	
	return 0;
}
