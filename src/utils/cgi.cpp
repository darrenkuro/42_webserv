#include "cgi.hpp"

#include <cstring>			// strcpy
#include <cstdlib>			// exit, WIFEXITED, WEXITSTATUS
#include <unistd.h>			// pipe, fork, chdir, dup2, close, execve, write, read
#include <sys/wait.h>		// waitpid

#include "http.hpp"			// createHttpResponse
#include "utils.hpp"		// toString, getFileExtension, fullPath
#include "Logger.hpp"		// Logger

/* ============================================================================================== */
/*                                                                                                */
/*                                 CGI Functional Implementation                                  */
/*                                                                                                */
/* ============================================================================================== */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Main Handler ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
HttpResponse processCgiRequest(HttpRequest req, const Client& client, const Server& server)
{
	try {
		StringMap envMap = getCgiEnv(req, client, server);

		// Check file exists and is executable
		ifstream file(envMap["PATH_TRANSLATED"].c_str());
		if (!file.good()) {
			return createHttpResponse(404, server.getErrorPage(404));
		}

		return createHttpResponse(executeCgi(envMap, req.body));
	}
	catch (const exception& e) {
		LOG_DEBUG << "CGI exception: " << e.what();
		return createHttpResponse(500, server.getErrorPage(500));
	}
}

/* ---------------------------------------------------------------------------------------------- */
string executeCgi(const StringMap& envMap, const string& reqBody)
{
	int pipeIn[2];
	int pipeOut[2];
	pid_t pid;

	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
		throw runtime_error("pipe() failed");
	}

	if ((pid = fork()) == -1) {
		throw runtime_error("fork() failed");
	}

	string result= "";
	if (pid == 0) {
		try {
			// Memory allocated inside child process, no need to free
			char** argv = getArgvPointer(envMap);
			char** env = getEnvPointer(envMap);

			// Change to the correct directory for relative path file access
			if (chdir(fullPath(ROOT, CGI_BIN).c_str()) == -1) {
				throw runtime_error("chdir() failed");
			}

			dup2(pipeIn[0], 0), dup2(pipeOut[1], 1);
			close(pipeIn[1]), close(pipeOut[0]);

			if (execve(argv[0], argv, env) == -1) exit(1);
		}
		catch (const exception& e) {
			LOG_DEBUG << "CGI child process exception: " << e.what();
			exit(1);
		}
	}
	else {
		int status;

		// Pipe stdin
		close(pipeOut[1]), close(pipeIn[0]);
		write(pipeIn[1], reqBody.c_str(), reqBody.size());
		close(pipeIn[1]);

		// Pipe stdout
		char buffer[RECV_SIZE];
		ssize_t bytesRead;
		while((bytesRead = read(pipeOut[0], buffer, RECV_SIZE)) > 0) {
			result += string(buffer, bytesRead);
		}
		close(pipeOut[0]);

		cout << result << endl;
		waitpid(pid, &status, 0);
		if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
			throw runtime_error("child process error");
		}
	}
	return result;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Environmental variable Getters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
StringMap getCgiEnv(HttpRequest& req, const Client& client, const Server& server)
{
	StringMap metaVars;

	metaVars["CONTENT_LENGTH"] = req.body.size();
	metaVars["CONTENT_TYPE"] = req.header["Content-Type"];
	metaVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	metaVars["PATH_INFO"] = req.uri;
	metaVars["PATH_TRANSLATED"] = fullPath(ROOT, getScriptName(req.uri));
	metaVars["QUERY_STRING"] = getQueryString(req.uri);
	metaVars["REMOTE_ADDR"] = toIpString(client.getClientIp());
	metaVars["REQUEST_METHOD"] = req.method;
	metaVars["SCRIPT_NAME"] = getScriptName(req.uri);
	metaVars["SERVER_NAME"] = server.getName();
	metaVars["SERVER_PORT"] = toString(server.getAddress().port);
	metaVars["SERVER_PROTOCOL"] = HTTP_VERSION;
	metaVars["SERVER_SOFTWARE"] = SERVER_SOFTWARE;

	return metaVars;
}

/* ---------------------------------------------------------------------------------------------- */
string getScriptName(const string& uri)
{
	if (uri.find(CGI_BIN) != 0) {
		throw runtime_error("no " + string(CGI_BIN) + " at the start");
	}

	size_t end = uri.find_first_of("/", string(CGI_BIN).size());
	return uri.substr(0, end);
}

/* ---------------------------------------------------------------------------------------------- */
string getQueryString(const string& uri)
{
	// Consider URL encoding?
	size_t sepPos = uri.find_first_of("?");
	return sepPos == string::npos ? "" : uri.substr(sepPos + 1);
}

/* ---------------------------------------------------------------------------------------------- */
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

/* ---------------------------------------------------------------------------------------------- */
char** getArgvPointer(const StringMap& envMap)
{
	// Validate CGI before memory allocation
	StringMap::const_iterator it = envMap.find("PATH_TRANSLATED");
	if (it == envMap.end()) {
		throw runtime_error("couldn't find PATH_TRANSLATED");
	}

	string ext = getFileExtension(it->second);
	if (ext != ".py" && ext != ".php") {
		throw runtime_error("extension " + ext + " not supported");
	}

	char** argvPointer = new char*[3];
	string execPath = ext == ".py" ? PY_PATH : PHP_PATH;
	string scriptPath = it->second.substr(it->second.find_last_of("/") + 1);

	argvPointer[0] = new char[20];
	std::strcpy(argvPointer[0], execPath.c_str());
	argvPointer[1] = new char[it->second.size() + 1];
	std::strcpy(argvPointer[1], scriptPath.c_str());
	argvPointer[2] = NULL;
	return argvPointer;
}
