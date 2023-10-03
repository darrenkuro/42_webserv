#pragma once

#include <set>
#include <map>
#include <deque>
#include <vector>
#include <iostream>
#include <netinet/in.h>

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::set;
using std::deque;
using std::vector;
using std::string;
using std::ostream;
using std::exception;
using std::runtime_error;

typedef struct pollfd PollFd;
typedef struct stat Stat;
typedef map<string, string> StringMap;
typedef vector<string, string> StringVec;

struct Address
{
	in_addr_t ip;
	int port;
};

struct HttpRequest
{
	string method;
	string uri;
	string version;
	string body;
	StringMap header;
};

struct HttpResponse
{
	int statusCode;
	string body;
	StringMap header;
};

struct LocationConfig
{
	bool autoindex;
	string uri;
	string alias;
	StringVec allowedMethods;
	StringVec index;
	std::pair<int, string> redirect;
};

struct ServerConfig
{
	string root;
	Address address;
	StringVec index;
	string serverName;
	ssize_t clientMaxBodySize;
	map<int, string> errorPages;
	vector<LocationConfig> locations;
};

#define DEFAULT_400		"./public/default_error/400.html"
#define DEFAULT_404		"./public/default_error/404.html"
#define DEFAULT_CONF	"config/default.conf"
#define ROOT			"./public"

#define RECV_SIZE		4096
#define CLIENT_TIMEOUT	60

#define RESET	"\033[0m"
#define CYAN	"\033[36m"
#define YELLOW	"\033[33m"
#define RED		"\033[31m"
#define ORANGE	"\033[38;5;214m"
#define DORANGE "\033[37;5;166m"

#define PADDING	21
#define DEBUG	4
#define INFO	3
#define WARNING	2
#define ERROR	1

#define DISPLAY_LEVEL	DEBUG
