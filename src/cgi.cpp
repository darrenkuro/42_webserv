#include "cgi.hpp"
#include "utils.hpp"
#include "log.hpp"
#include <cstring>
#include <cstdlib>	// exit
#include <sys/wait.h>		// waitpid
#include "http.hpp"			// createHttpResponse
#include "ConfigParser.hpp"	// isValidErrorCode

string getScriptName(const string& uri)
{
	if (uri.find(CGI_BIN) != 0) {
		throw runtime_error("no " + string(CGI_BIN) + " at the start");
	}

	size_t end = uri.find_first_of("/", string(CGI_BIN).size());
	return uri.substr(0, end);
}

string getQueryString(const string& uri)
{
	// Consider URL encoding?
	size_t sepPos = uri.find_first_of("?");
	return sepPos == string::npos ? "" : uri.substr(sepPos + 1);
}

StringMap getCgiEnv(HttpRequest& req, const Client& client, const Server& server)
{
	StringMap metaVars;

	metaVars["CONTENT_LENGTH"] = req.header["Content-Length"];
	metaVars["CONTENT_TYPE"] = req.header["Content-Type"];
	metaVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	metaVars["PATH_INFO"] = req.uri;
	metaVars["PATH_TRANSLATED"] = fullPath(ROOT, getScriptName(req.uri));
	metaVars["QUERY_STRING"] = getQueryString(req.uri);
	metaVars["REMOTE_ADDR"] = ""; // MUST, need getpeername??
	metaVars["REQUEST_METHOD"] = req.method;
	metaVars["SCRIPT_NAME"] = getScriptName(req.uri);
	metaVars["SERVER_NAME"] = server.getName();
	metaVars["SERVER_PORT"] = toString(server.getAddress().port);
	metaVars["SERVER_PROTOCOL"] = HTTP_VERSION;
	metaVars["SERVER_SOFTWARE"] = SERVER_SOFTWARE;
	(void) client;

	return metaVars;
}

char** getEnvPointer(const StringMap& envMap)
{
	char** envPointer = new char*[envMap.size() + 1];

	size_t i = 0;
	for (StringMap::const_iterator it = envMap.begin(); it != envMap.end(); it++) {
		string envString = it->first + "=" + it->second;
		envPointer[i] = new char[envString.size() + 1];
		std::strcpy(envPointer[i], envString.c_str());
		i++;
	}
	envPointer[envMap.size()] = NULL;
	return envPointer;
}

char** getArgvPointer(const StringMap& envMap)
{
	// Validate CGI before memory allocation
	StringMap::const_iterator it = envMap.find("PATH_TRANSLATED");
	if (it == envMap.end()) {
		throw runtime_error("couldn't find PATH_TRANSLATED");
	}

	string ext = getExtension(it->second);
	if (ext != ".py" && ext != ".php") {
		throw runtime_error("extension " + ext + " not supported");
	}

	char** argvPointer = new char*[3];
	string execPath = ext == ".py" ? PY_PATH : PHP_PATH;

	argvPointer[0] = new char[20];
	std::strcpy(argvPointer[0], execPath.c_str());
	argvPointer[1] = new char[it->second.size() + 1];
	std::strcpy(argvPointer[1], it->second.c_str());
	argvPointer[2] = NULL;
	return argvPointer;
}

// void freePointer(char** ptr)
// {
//     if (!ptr) return;

//     for (char** i = ptr; *i != NULL; ++i) {
//         delete[] *i;
//     }
//     delete[] ptr;
// }

string executeCgi(const StringMap& envMap, int& code)
{
	int fd[2];
	pid_t pid;

	if (pipe(fd) == -1) {
		log(ERROR, "pipe() failed");
		code = 500;
		return "";
	}

	if ((pid = fork()) == -1) {
		log(ERROR, "fork() failed");
		code = 500;
		return "";
	}

	string result= "";
	if (pid == 0) {
		try {
			char** argv = getArgvPointer(envMap);
			char** env = getEnvPointer(envMap);

			dup2(fd[1], 1);
			close(fd[0]), close(fd[1]);
			if (execve(argv[0], argv, env) == -1) exit(1);
		}
		catch (exception& e) {
			log(ERROR, "CGI child process exception: " + string(e.what()) + "!");
			exit(1);
		}
	}
	else {
		int status;
		close(fd[1]);

		char buffer[RECV_SIZE];
		ssize_t bytesRead;
		while((bytesRead = read(fd[0], buffer, RECV_SIZE)) > 0) {
			result += string(buffer);
		}

		// Is it okay that the parent wait for the child? Still non-blocking?
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			code = 200;
			// success, data handling for HTTP
			cout << "CGI script executed successfully." << endl;
		} else {
			// failure, return something else
			cerr << "CGI script execution failed." << endl;
		}
	}
	return result;
}

HttpResponse processCgiRequest(HttpRequest req, const Client& client, const Server& server)
{
	try {
		int code;
		StringMap envMap = getCgiEnv(req, client, server);
		string output = executeCgi(envMap, code);

		// check file exists, and has permission?

		if (code == 200) return createHttpResponse(output);
		if (ConfigParser::isValidErrorCode(code)) {
			return createHttpResponse(code, server.getErrorPage(code));
		}

		// if inside errorcode, create basic response
		// else return string
		return createHttpResponse(500, server.getErrorPage(500));
	}
	catch (exception& e) {
		log(WARNING, "CGI process exception:" + string(e.what()) + "!");
		return createHttpResponse(500, server.getErrorPage(500));
	}
}