#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

#define EPOLL_SIZE 100

void echo(int sock, struct sockaddr_in addr)
{
    int nbytes_read;
    char buffer[BUFSIZ];

    // Does it return only when receive data? So In normal case, read() returns always greater than 0?
    if ((nbytes_read = read(sock, buffer, BUFSIZ)) > 0)
    {
        printf("received %d bytes:\n", nbytes_read);
        write(STDOUT_FILENO, buffer, nbytes_read);
        write(sock, buffer, nbytes_read);
    }
    else
    {
        printf("Client closed\n");
        close(sock);
    }
}

int main(int argc, char **argv)
{
    struct protoent *protoent;
    int enable = 1;
    int i;
    int newline_found = 0;
    int server_sockfd, client_sockfd;
    socklen_t client_len;
    struct sockaddr_in client_address, server_address;
    int server_port = 9090;

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    protoent = getprotobyname("tcp");
    if (protoent == NULL)
    {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }

    server_sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (server_sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // What is reuseaddr?
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY?
    server_address.sin_port = htons(server_port);       // Why do it use htons?
    if (bind(server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 5 is backlog. What is backlog?
    if (listen(server_sockfd, 5) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("listening on port %d\n", server_port);

    // epoll 인스턴스 생성
    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = server_sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_sockfd, &event);

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == server_sockfd)
            {

                client_len = sizeof(client_address);
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

                event.events = EPOLLIN;
                event.data.fd = client_sockfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &event);
            }
            else
            {
                echo(ep_events[i].data.fd, client_address);
            }
        }
    }
    return 0;
}