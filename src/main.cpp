#include "Webserver.hpp"
#include "utils.hpp"
#include "log.hpp"
#include <signal.h>

#define ERR_ARG			"Wrong number of arguments!"
#define DEFAULT_CONF	"configs/default.conf"

bool g_running = true;

//-----------------------------------------------------------------------------------------


void signalHandler(int signum) {
	if (signum == SIGINT) {
		g_running = false;
		std::cout << std::endl;
	}
}

int main(int argc, char **argv)
{
	try {
		if (argc > 2) {
			log(ERROR, ERR_ARG);
			return 1;
		}

		signal(SIGINT, signalHandler);
		ConfigParser parser;
		std::string config = argc == 2 ? argv[1] : DEFAULT_CONF;
		std::vector<ServerConfig> servers = parser.parse(config);

		Webserver webserver(servers);
		webserver.start();

	} catch (std::exception &e) {
		log(ERROR, e.what());
	}

	return 0;
}
