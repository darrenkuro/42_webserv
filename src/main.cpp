#include "Webserver.hpp"
#include "utils.hpp"

#include <signal.h>

#define ERR_ARG "[Error] Wrong number of arguments!"

bool g_running;

//-----------------------------------------------------------------------------------------


void signalHandler(int signum) {
	if (signum == SIGINT) {
		g_running = false;;
		std::cout << std::endl;
	}
}

int main(int argc, char **argv)
{
	if (argc > 2) {
		std::cerr << ERR_ARG << std::endl;
		return 1;
	}
	signal(SIGINT, signalHandler);
	(void) argv;
	g_running = true;

	try {
		ConfigParser parser;
		std::vector<ServerConfig> servers = parser.parse(argv[1]);
		Webserver webserver(servers);
		webserver.start();
	} catch (std::exception &e) {
		log(ERROR, e.what());
	}
	// ServerConfig config1;
	// config1.port = 4242;
	// config1.serverName = "lego.com";

	// ServerConfig config2;
	// config2.port = 8088;
	// config2.serverName = "www.mouse.com";

	// vector<ServerConfig> servers;
	// servers.push_back(config1);
	// servers.push_back(config2);

	// Webserver webserver(servers);
	// webserver.start();


	// log(ERROR, "Some random Error message");
	// log(DEBUG, "Some random Debug message");
	// log(INFO, "Some random Info message");


	// std::string fcon = getFileContent("./public_html/index.html");
	// std::cout << fcon << std::endl;

	// WebservConfig config2;
	// config2.serverName = "Server2";
	// config2.port = 4244;
	// Webserv webserv1(config1);

	// webserv1.launch();
	// webserv2.launch();

	return 0;
}
