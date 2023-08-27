#include "Webserv.hpp"
#include "utils.hpp"

#define ERR_ARG "[Error] Wrong number of arguments!"

//-----------------------------------------------------------------------------------------

WebservConfig config1 = {"Server1", 4242};
Webserv webserv1(config1);

void signalHandler(int signum) {
	if (signum == SIGINT) {
		webserv1.stop();
	}
	exit(1);
}

int main(int argc, char **argv)
{
	if (argc > 2) {
		std::cerr << ERR_ARG << std::endl;
		return 1;
	}
	signal(SIGINT, signalHandler);
	(void) argv;

	std::string fcon = getFileContent("./public_html/index.html");
	std::cout << fcon << std::endl;

	// WebservConfig config2;
	// config2.serverName = "Server2";
	// config2.port = 4244;
	// Webserv webserv1(config1);

	webserv1.launch();
	//webserv2.launch();

	while (true) {}
	return 0;
}
