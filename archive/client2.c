#include <stdio.h>
#ifdef __WIN32__
#include <winsock2.h>
#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

int main() {
    printf("Hi Mom!\n");
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error1\n");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        printf("Error2\n");
    }

    char sendline[100];
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    int sendbytes = strlen(sendline);
    // if (write(sockfd, sendline, sendbytes) != sendbytes) {
    //     printf("Error3\n");
    // }

    char recvline[100000];
    memset(recvline, 0, 100000);
    // int recvbytes;
    // while ((recvbytes = read(sockfd, recvline, 100000 - 1)) > 0)
    //     printf("%s", recvline);


    send(sockfd, "GET / HTTP/1.1\r\n\r\n", 23, 0);
    // while (recv(sockfd, recvline, 100000 - 1, 0) > 0) {
    //     printf("%s", recvline);
    // }

    printf("End\n");
    close(sockfd);
}
