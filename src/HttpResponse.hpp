#pragma once

#include <string>
#include <map>


class HttpResponse {
public:
	HttpResponse();
	~HttpResponse();
	HttpResponse(const HttpResponse &rhs);
	HttpResponse &operator=(const HttpResponse &rhs);

	static std::map<int, std::string> statusMap;
	
private:
	std::string m_version;
	int m_statusCode;
	std::string m_statusText;
	std::map<std::string, std::string> m_header;
	std::string m_body;

};

struct HttpResponse
{
	std::string version;
	int statusCode;
	std::string statusText;
	std::map<std::string, std::string> header;
	std::string body;
};



1. Identify method -> call appropiate function 

Get control flow:
    - check if http version is supported by our server
    - url parsing. properly encoded etc, no invalid characetrs 
    - is url valid? Does that path exist for our webpage
    - ?maybe do query parameters

    - validate any headers that are accepted by our server (relevant to our server functionality)

    info: validation can maybe be done by the http parser. shoudlnt be in http request builder i think

    even if something failed we need to call some build function. we can build error http response




Lets try serving this

GET /index.html HTTP/1.1
Host: example.com


HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 53

<!DOCTYPE html>
<html>
<head>
    <title>Minimal Webpage</title>
</head>
<body>
    <h1>Hello, World!</h1>
</body>
</html>