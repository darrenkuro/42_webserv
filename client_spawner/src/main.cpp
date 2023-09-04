#include "ClientC.hpp"

int main()
{
	ClientC client1;
	ClientC client2;
	ClientC client3;
	ClientC client4;

	sleep(4);

	client1.connect("127.0.0.1", 4422);
	// client2.connect("127.0.0.1", 8088);
	// sleep(1);
	client1.sendRequest("GET /about/me HTTP/1.1\r\nHost:127.0.0.1:4422\r\n\r\n");
	// sleep(3);
	// client2.sendRequest("GET /about/me HTTP/1.1\r\nHost:www.tutorialspoint.com\r\nAccept-Language:   en-us\r\nConnection:    Keep-Alive\r\n\r\n");
	// client1.sendRequest("GET /about/me HTTP/1.1\r\nHost:www.tutorialspoint.com\r\nAccept-Language:   en-us\r\nConnection:    Keep-Alive\r\n\r\n");
	// client3.connect("127.0.0.1", 8088);
	// sleep(3);
	sleep(50);
	// client4.connect("127.0.0.1", 4243);
	// client4.sendRequest("GET /about/me HTTP/1.1\r\nHost:www.tutorialspoint.com\r\nAccept-Language:   en-us\r\nConnection:    Keep-Alive\r\n\r\n");
	// sleep(10);
	// client1.sendRequest("GET /about/me HTTP/1.1\r\nHost:www.tutorialspoint.com\r\nAccept-Language:   en-us\r\nConnection:    Keep-Alive\r\n\r\n");
	// client2.sendRequest("GET /about/me HTTP/1.1\r\nHost:www.tutorialspoint.com\r\nAccept-Language:   en-us\r\nConnection:    Keep-Alive\r\n\r\n");
	// client2.sendRequest("GET /about/me HTTP/1.1\r\nHost:www.tutorialspoint.com\r\nAccept-Language:   en-us\r\nConnection:    Keep-Alive\r\n\r\n");
	// client1.disconnect();
	// client4.disconnect();
	// sleep(20);
	// client2.sendRequest("GET /about/me HTTP/1.1\r\nHost:www.tutorialspoint.com\r\nAccept-Language:   en-us\r\nConnection:    Keep-Alive\r\n\r\n");
	// client2.disconnect();
}
