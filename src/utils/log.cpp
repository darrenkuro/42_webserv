#include "log.hpp"
#include "utils.hpp"	// toIpString
#include <ctime>		// time_t, time, ctime
#include <cstdio>		// vfprintf

ostream& displayTimestamp(ostream& os)
{
	time_t result = std::time(NULL);
	string timestamp(std::ctime(&result));
	os << CYAN << "[" + timestamp.substr(11, 8) + "]" << RESET;
	return os;
}

ostream& align(ostream& os)
{
	string padding(PADDING, ' ');
	os << padding;
	return os;
}

void displayLogLevel(int level)
{
	switch (level)
	{
	case DEBUG:
		cout << YELLOW << "[DEBUG]" << RESET << "   ";
		break;

	case INFO:
		cout << CYAN << "[INFO]" << RESET << "    ";
		break;

	case WARNING:
		cout << RED << "[WARNING]" << RESET << " ";
		break;

	case ERROR:
		cerr << RED << "[ERROR]" << RESET << "   ";
		break;
	}
}

void log(int level, const char* format, ...)
{
	if (level > LOG_DISPLAY_LEVEL) return;
	if (level == ERROR)	cerr << displayTimestamp << " ";
	else cout << displayTimestamp << " ";
	displayLogLevel(level);

	va_list args;
	va_start(args, format);
	if (level == ERROR) vfprintf(stderr, format, args);
	else vfprintf(stdout, format, args);
	va_end(args);

	if (level == ERROR) cerr << endl;
	else cout << endl;
}

void log(int level, const string& str)
{
	if (level > LOG_DISPLAY_LEVEL) return;
	cout << displayTimestamp << " ";
	displayLogLevel(level);

	cout << str << endl;
}

void log(ServerConfig config)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << "ServerConfig: ";
	cout << config;
}

void log(HttpRequest req, int clientID)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << ORANGE;
	cout << "HTTP Req>> Client[ID " << clientID << "]" << endl;
	cout << req << RESET << endl;
}

void log(HttpResponse res, int clientID)
{
	cout << displayTimestamp << " ";
	displayLogLevel(INFO);

	cout << ORANGE;
	cout << "HTTP <<Res Client[ID " << clientID << "]" << endl;
	cout << res << RESET << endl;
}

ostream& operator<<(ostream& os, HttpRequest req)
{
	os << align;
	os << "Method[" << req.method << "] ";
	os << "Uri[" << req.uri << "] ";
	os << "Version[" << req.version << "] ";
	os << "Host[" << req.header.find("Host")->second << "]" << endl;
	os << align;
	os << "Body[" + req.body + "]";
	return os;
}

ostream& operator<<(ostream& os, HttpResponse res)
{
	os << align;
	os << "Status[" << res.statusCode << "]";
	return os;
}

ostream &operator<<(ostream &os, const Address address)
{
	os << "Address[" << toIpString(address.ip) << ":";
	os << address.port << "]";
	return os;
}

ostream &operator<<(ostream &os, const LocationConfig location)
{
	os << "URI[" << location.uri << "] ";
	os << "Alias[" << location.alias << "] ";
	os << "Autoindex[" << location.autoindex << "] ";
	os << "Redirect[" << location.redirect.first << ":";
	os << location.redirect.second << "] AllowedMethods[";

	vector<string>::const_iterator it;
	for (it = location.allowedMethods.begin(); it != location.allowedMethods.end(); it++) {
		os << " " << *it;
	}
	os << " ] Index[";
	for (it = location.index.begin(); it != location.index.end(); it++) {
		os << " " << *it;
	}
	os << " ]";
	return os;
}

ostream &operator<<(ostream &os, const ServerConfig config)
{
	os << "ServerName[" << config.serverName << "] ";
	os << "Root[" << config.root << "] ";
	os << "MaxBodySize[" << config.clientMaxBodySize << "] ";
	os << config.address << endl;

	os << align, os << "Locations: " << endl;
	vector<LocationConfig>::const_iterator it2;
	for (it2 = config.locations.begin(); it2 != config.locations.end(); it2++) {
		os << align;
		os << ">> " << *it2 << endl;
	}

	os << align, os << "ErrorPages: ";
	map<int, string>::const_iterator it;
	int i = 0;
	for (it = config.errorPages.begin(); it != config.errorPages.end(); it++) {
		if (i++ % 3 == 0) {
			os << endl, os << align, os << ">> ";
		}
		os << it->first << "[" << it->second << "] ";
	}
	os << endl;
	return os;
}
