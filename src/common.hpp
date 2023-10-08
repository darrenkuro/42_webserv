#pragma once

/* Headers */
#include <set>			// set
#include <map>			// map
#include <deque>		// deque
#include <vector>		// vector
#include <fstream>		// ifstream, ofstream
#include <sstream>		// stringstream
#include <iostream>		// cout, cerr, endl
#include <exception>	// exception
#include <stdexcept>	// runtime_error
#include <netinet/in.h>	// in_addr_t, htonl, htons, ntohl, ntohs

/* Using */
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::set;
using std::deque;
using std::vector;
using std::string;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::istringstream;
using std::ostringstream;
using std::exception;
using std::runtime_error;

/* Typedefs*/
typedef struct pollfd PollFd;
typedef struct stat Stat;
typedef struct sockaddr_in Sockaddr_in;
typedef struct sockaddr Sockaddr;
typedef map<string, string> StringMap;
typedef vector<string> StringVec;

/* Data Structures */
struct Address
{
	in_addr_t ip;
	int port;
	bool operator<(const Address& rhs) const
	{
		if (ip == rhs.ip)
			return port < rhs.port;
		return ip < rhs.ip ? true : false;
	}
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

/* Paths */
#define DEFAULT_400		"public/default_error/400.html"
#define DEFAULT_404		"public/default_error/404.html"
#define DEFAULT_CONF	"config/default.conf"
#define ROOT			"public"
#define CGI_BIN			"/cgi-bin/"
#define PY_PATH			"/usr/bin/python3"
#define PHP_PATH		"/usr/bin/php"

/* Server Params */
#define HTTP_VERSION	"HTTP/1.1"
#define SERVER_SOFTWARE	"Webserv42/1.0"

/* Setting Params */
#define RECV_SIZE			4096
#define CLIENT_TIMEOUT		60
#define LOG_DISPLAY_LEVEL	DEBUG
