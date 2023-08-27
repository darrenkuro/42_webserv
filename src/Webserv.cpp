#include "Webserv.hpp"
#include "utils.hpp"

#define LOG(x) std::cout << x << std::endl;
#define LOGERR(x) std::cerr << x << std::endl;

#include <unistd.h>
#include <vector>

Webserv::Webserv() : m_config() {}
Webserv::~Webserv() {}
Webserv::Webserv(const WebservConfig config) : m_config(config) {}
Webserv::Webserv(const Webserv &rhs) : m_config(rhs.m_config) {*this = rhs;}
Webserv &Webserv::operator=(const Webserv &rhs)
{
	if (this == &rhs)
		return *this;
	// m_config = rhs.m_config;
	return *this;
}

//-----------------------------------------------------------------------------------------
void Webserv::initSocket()
{
	m_listeningSock = createIPv4Socket();
	m_serverAddr = createAddress(m_config.port);

	if (bind(m_listeningSock, (struct sockaddr *)&m_serverAddr, sizeof(m_serverAddr)) < 0) {
		throw std::runtime_error("[Error] Bind failed!");
	}

	if (listen(m_listeningSock, 10)) {
		throw std::runtime_error("[Error] Listen failed!");
	}

	struct pollfd listeningFd;
	listeningFd.fd = m_listeningSock;
	listeningFd.events = POLLIN;
	m_pollFds.push_back(listeningFd);
}

//-----------------------------------------------------------------------------------------
void Webserv::launch()
{
	int pid = fork();
	if (pid == -1) {
		LOGERR("[Error] Fork failed!");
		return;
	}
	if (pid == 0) {
		try {
			initSocket();
			LOG(m_config.serverName + " launched successfully");

			mainloop();

		} catch (const std::exception &e) {
			std::cerr << e.what() << std::endl;
			exit(1);
		}
		exit(0);
	}
	m_pid = pid;
}

//-----------------------------------------------------------------------------------------
void Webserv::mainloop()
{
	while (true) {
		int pollReady = poll(m_pollFds.data(), m_pollFds.size(), 1000);
		if (pollReady == -1) {
			throw std::runtime_error("[Error] Poll failed!");
		}

		if (m_pollFds[0].revents & POLLIN) {
			connectNewClient();
		}
		
		for (size_t i = 1; i < m_pollFds.size(); i++) {
			if (m_pollFds[i].revents & POLLIN) {
				char buffer[RECV_SIZE];
				ssize_t bytesRead = recv(m_pollFds[i].fd, buffer, RECV_SIZE, 0);
				if (bytesRead <= 0) {
					clientErrorHandling(bytesRead, i);
				}
				else {
					processClientEvents(i);
				}
			}
		}
		cleanupDisconnClients();
	}
}

//-----------------------------------------------------------------------------------------
void Webserv::connectNewClient()
{
	// Could save client address but not necessary for now
	int newClient = accept(m_listeningSock, nullptr, nullptr);
	m_pollFds.push_back({newClient, POLLIN, 0});
}

//-----------------------------------------------------------------------------------------
void Webserv::clientErrorHandling(ssize_t bytesRead, int clientIdx)
{
	if (bytesRead == 0) {
		m_disconnClientIdxs.push_back(clientIdx);
	}
	else {
		// Error or connection closed
	}
}

//-----------------------------------------------------------------------------------------
void Webserv::processClientEvents(int idx)
{

}

//-----------------------------------------------------------------------------------------
void Webserv::cleanupDisconnClients()
{
	for (int i = 0; i < m_disconnClientIdxs.size(); i++) {
		int dynamicClientIdx = m_disconnClientIdxs[i] - i;
		close(m_pollFds[dynamicClientIdx].fd);
		m_pollFds.erase(m_pollFds.begin() + dynamicClientIdx);
	}
	m_disconnClientIdxs.clear();
}

//-----------------------------------------------------------------------------------------
void Webserv::stop()
{
	if (kill(m_pid, SIGTERM) == -1) {
		LOG(m_config.serverName + " stop failed!");
		return;
	}
	close(m_listeningSock);
	LOG(m_config.serverName + " stopped");

	// Also should do cleanup in here. Closing pollFds
}







// //-----------------------------------------------------------------------------------------
// void Webserv::polling()
// {
// 	int pollRes = poll(m_pollFds.data(), m_pollFds.size(), 1000);
// 	if (pollRes == -1) {
// 		throw std::runtime_error("[Error] Poll failed!");
// 	} else if (pollRes == 0) {
// 		//std::cerr << "Poll timeout" << std::endl;
// 	} else {
// 		if (m_pollFds[0].revents & POLLIN) {
// 			struct sockaddr_in client_addr;
// 			socklen_t length = sizeof(client_addr);
// 			int newClient = accept(m_sockfd, (struct sockaddr *)&client_addr, &length);
// 			connectClient(newClient);
// 		}
// 		for (pollIt it = m_pollFds.begin() + 1; it != m_pollFds.end();) {
// 		//for(std::vector<struct pollfd>::size_type i = 1; i < m_pollFds.size(); i++) {
// 			char recvline[RECV_SIZE];
// 			ssize_t recvBytes;
// 			if (it->revents & POLLIN) {
// 				recvBytes = recv(it->fd, recvline, RECV_SIZE - 1, 0);
// 				std::string recvStr(recvline);
// 				if (recvBytes == -1) {
// 					throw std::runtime_error("[Error] Recv failed!");
// 				} else if (recvBytes == 0) {
// 					disconnectClient(it);
// 				} else {
// 					try {
// 						HttpRequest req = parseHttpRequest(recvStr);
// 						std::cout << req.method << std::endl;
// 						std::cout << req.url << std::endl;
// 						std::cout << req.version << std::endl;
// 						for (std::map<std::string, std::string>::iterator it = req.header.begin(); it != req.header.end(); it++) {
// 							std::cout << it->first << ": " << it->second << std::endl;
// 						}
// 					} catch (std::exception &e) {
// 						std::cerr << "[Error] Invalid HTTPRequest!" << std::endl;
// 					}
// 					++it;
// 				}
// 			} else {
// 				++it;
// 			}
// 		}
// 	}
// 	//std::cout << "Looping" << std::endl;
// }

// void Webserv::connectClient(int clientFd)
// {
// 	struct pollfd newClient;
// 	newClient.fd = clientFd;
// 	newClient.events = POLLIN | POLLOUT;
// 	m_pollFds.push_back(newClient);
// 	std::cout << "Client connected" << std::endl;
// }

// void Webserv::disconnectClient(pollIt &client)
// {
// 	client = m_pollFds.erase(client);
// 	std::cout << "Client disconnected" << std::endl;
// }