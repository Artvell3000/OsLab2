#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRES "127.0.0.1"
#define SERVER_PORT 1234

char* randStr(int lim) {
    int sizeS = rand() % lim;
    char* s = new char[sizeS];
    for (int i = 0;i < (sizeS-1);i++) {
        s[i] = 'a' + rand() % ('z' - 'a');
    }
    s[sizeS - 1] = '\0';
    return s;
}

int main(int argv, char** argc)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
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
        char* buf = randStr(20);

        int sendResult = send(socketClient, buf, sizeof(buf), 0);
        if (sendResult < 0) {
            perror("send!!!");
        }
        printf("serdResult: %d \n", sendResult);
        sleep(1);
    }

    shutdown(socketClient, SHUT_RDWR);
    close(socketClient);

}