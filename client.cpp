#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRES "127.0.0.1"
#define SERVER_PORT 12345

int main(int argv, char** argc)
{
    int socketClient = socket(AF_INET,SOCK_STREAM,0);

    if (socketClient < 0) {
        perror("socket");
    }

    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_ADDRES, &sockAddr.sin_addr);
    sockAddr.sin_port = htons(SERVER_PORT);

    int connectResult = connect(socketClient, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
    if (connectResult < 0) {
        perror("connect!!!");
    }
    while (1) {
        char buf[] = "sms";
        int sendResult = send(socketClient, buf, sizeof(buf), 0);
        if (sendResult < 0) {
            perror("send!!!");
        }
        sleep(1);
    }

    shutdown(socketClient, SHUT_RDWR);
    close(socketClient);

}