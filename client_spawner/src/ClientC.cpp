#include "ClientC.hpp"

using std::string;

ClientC::ClientC() {}
ClientC::~ClientC() {}

void ClientC::connect(string ipAddress, int port)
{
    if ((m_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error1\n");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

    if (::connect(m_socketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error2\n");
    }
}

void ClientC::disconnect()
{
    close(m_socketFd);
}

void ClientC::sendRequest(string request)
{
    send(m_socketFd, request.c_str(), request.size(), 0);
}
