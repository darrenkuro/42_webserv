#include "utils.hpp"

#include <climits>		// INT_MAX, INT_MIN

#include "Logger.hpp"	// Logger

/* ============================================================================================== */
/*                                                                                                */
/*                                  General Utils Implementation                                  */
/*                                                                                                */
/* ============================================================================================== */

/* --------------------------------------------------------------------------------------------- */
bool isAllDigit(const string& str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i])) return false;
	}
	return true;
}

/* ---------------------------------------------------------------------------------------------- */
string toString(int value)
{
	stringstream ss;
	ss << value;

	// Unlikely to happen, bad alloc only
	if (ss.fail()) return "";

	return ss.str();
}

/* ---------------------------------------------------------------------------------------------- */
string toIpString(in_addr_t ip)
{
	ostringstream oss;
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&ip);

	oss << static_cast<int>(bytes[0]) << '.'
		<< static_cast<int>(bytes[1]) << '.'
		<< static_cast<int>(bytes[2]) << '.'
		<< static_cast<int>(bytes[3]);

	return oss.str();
}


/* ---------------------------------------------------------------------------------------------- */
string getFileContent(const string& path)
{
	ifstream file(path.c_str());
	if (!file.is_open()) {
		throw runtime_error("failed to open file " + path);
	}

	string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

/* ---------------------------------------------------------------------------------------------- */
string getFileExtension(const string& path)
{
	size_t pos = path.find_last_of('.');

	if (pos != string::npos && pos != 0) return path.substr(pos);
	return "";
}

/* ---------------------------------------------------------------------------------------------- */
string fullPath(string root, string path)
{

	try {
		// Remove '/' from root and path
		root = root.at(root.size() - 1) == '/' ? root.substr(0, root.size() - 1) : root;
		path = path.at(0) == '/' ? path.substr(1) : path;
	}
	catch (const exception& e) {
		// Protect against if strings are empty or index out of bound
		LOG_DEBUG << "FullPath exception: " << e.what();
		return "";
	}
	return root + "/" + path;
}

/* ---------------------------------------------------------------------------------------------- */
string getBoundary(const HttpRequest& req)
{
	StringMap::const_iterator it;
	it = req.header.find("Content-Type");
	if (it == req.header.end()) {
		throw runtime_error("couldn't find Content-Type header");
	}

	size_t pos = it->second.find("boundary=");
	if (pos == string::npos) {
		throw runtime_error("couldn't find 'boundary='");
	}

	return "--" + it->second.substr(pos + 9);
}

/* ---------------------------------------------------------------------------------------------- */
int toInt(const string& str)
{
	istringstream iss(str);
	int result;
	char remainingChar;

	// Check if there are leftover characters
	if (!(iss >> result) || (iss.get(remainingChar))) {
		throw runtime_error("convert " + str + " toInt fails");
	}

	// Check fail and edge cases
	if (result == 0 && str != "0" && str != "+0" && str != "-0") {
		throw runtime_error("convert " + str + " toInt fails");
	}
	if (result == INT_MAX && str != "2147483647" && str != "+2147483647") {
		throw runtime_error("convert " + str + " toInt fails");
	}
	if (result == INT_MIN && str != "-2147483648") {
		throw runtime_error("convert " + str + " toInt fails");
	}

	return result;
}

/* ---------------------------------------------------------------------------------------------- */
in_addr_t toIpNum(string str)
{
	in_addr_t result = 0;

	for (int i = 0; i < 4; i++) {
		if (i < 3 && str.find('.') == string::npos) {
			throw runtime_error("failed to convert " + str);
		}

		string token = i < 3 ? str.substr(0, str.find('.')) : str;
		int value = toInt(token);
		if (!isAllDigit(token) || value < 0 || value > 255) {
			throw runtime_error("failed to convert " + str);
		}

		result = (result << 8) | value;
		if (i < 3) str.erase(str.begin(), str.begin() + str.find('.') + 1);
	}

	return htonl(result);
}

/*
ostream& operator<<(ostream& os, HttpRequest req)
{
	os << "Method[" << req.method << "] ";
	os << "Uri[" << req.uri << "] ";
	return os;
}

ostream& operator<<(ostream& os, HttpResponse res)
{
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

	//os << align;
	os << "Locations: " << endl;
	vector<LocationConfig>::const_iterator it2;
	for (it2 = config.locations.begin(); it2 != config.locations.end(); it2++) {
		//os << align;
		os << ">> " << *it2 << endl;
	}

	//os << align
	os << "ErrorPages: ";
	map<int, string>::const_iterator it;
	int i = 0;
	for (it = config.errorPages.begin(); it != config.errorPages.end(); it++) {
		if (i++ % 3 == 0) {
			os << endl;
			//os << align
			os << ">> ";
		}
		os << it->first << "[" << it->second << "] ";
	}
	os << endl;
	return os;
}
*/
