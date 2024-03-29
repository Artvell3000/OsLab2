#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <locale.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/time.h>

#define PORT_NUMBER 1234
#define BUFFER_LENGTH 1024
#define MAX_EVENTS 32
#define TEST

volatile sig_atomic_t sighup = 0;

void sigHandler(int r)
{
    sighup = 1;
}

int setNonblock(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IOLBF, 0);

#if defined(TEST)
    printf("start prog\n");
#endif
    
    setlocale(LC_ALL, "ru");

#if defined(TEST)
    printf("setlocale()\n");
#endif

    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);


#if defined(TEST)
    printf("sigaction()\n");
#endif
    
    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

#if defined(TEST)
    printf("sigprocmask()\n");
#endif

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    // сделаем сокет неблокирующим 
    setNonblock(serverSocket);

    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(PORT_NUMBER);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((bind(serverSocket, (struct sockaddr*)&sockAddr, sizeof(sockAddr))) < 0)
    {
        perror("Bind");
        exit(EXIT_FAILURE);
    }

    listen(serverSocket, SOMAXCONN);

    // создам дескриптор epoll
    int ePoll = epoll_create1(0);
    struct epoll_event event;
    // зарегестрируем сервер в epoll
    event.data.fd = serverSocket;
    event.events = EPOLLIN;
    epoll_ctl(ePoll, EPOLL_CTL_ADD, serverSocket, &event);

    while (1)
    {
        struct epoll_event events[MAX_EVENTS];

#if defined(TEST)
        printf("epoll_wait\n");
#endif
        int quantityEvents = epoll_wait(ePoll, events, MAX_EVENTS, -1);
#if defined(TEST)
        printf("epoll_wait completed\n");
#endif
        
        if (quantityEvents < 0) {
            if (errno == EINTR) {
                printf("SIGHUB!!!");
                break;
            }
            else {
                perror("epoll_event!!!");
                break;
            }
        }

        for (size_t i = 0; i < quantityEvents; ++i)
        {
            if (events[i].data.fd == serverSocket)
            {
                int clientSocket = accept(serverSocket, 0, 0);
                if (clientSocket < 0)
                {
                    perror("Accept");
                    exit(EXIT_FAILURE);
                }
                setNonblock(serverSocket);

                //зарегистрируем клиентский сокет
                event.data.fd = clientSocket;
                event.events = EPOLLIN;
                epoll_ctl(ePoll, EPOLL_CTL_ADD, clientSocket, &event);
            }
            else
            {
                char buf[1024];
                int recvResult = recv(events[i].data.fd, buf, 1024, MSG_NOSIGNAL);
                printf("полученно %d : %s \n", recvResult, buf);
                if ((recvResult == 0) && (errno != EAGAIN)) {
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                }

            }
        }
    }
}