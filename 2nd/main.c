#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

struct thread_arg {
    pthread_t self;
    int sock;
    struct sockaddr_in addr;
};

void echo(int sock, struct sockaddr_in addr) {
    int nbytes_read;
    char buffer[BUFSIZ];

    // Does it return only when receive data? So In normal case, read() returns always greater than 0?
    while ((nbytes_read = read(sock, buffer, BUFSIZ)) > 0) {
        printf("received %d bytes:\n", nbytes_read);
        write(STDOUT_FILENO, buffer, nbytes_read);
        write(sock, buffer, nbytes_read);
    }
    printf("Client closed\n");
    close(sock);
}

void *thread_main(void *arg) {
    struct thread_arg *t_arg = (struct thread_arg *)arg;
    echo(t_arg->sock, t_arg->addr);
    free(arg);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    struct protoent *protoent;
    int enable = 1;
    int i;
    int newline_found = 0;
    int server_sockfd, client_sockfd;
    socklen_t client_len;
    struct sockaddr_in client_address, server_address;
    int server_port = 8080;

    protoent = getprotobyname("tcp");
    if (protoent == NULL) {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }

    server_sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (server_sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // What is reuseaddr?
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY?
    server_address.sin_port = htons(server_port); // Why do it use htons?
    if (bind(server_sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 5 is backlog. What is backlog?
    if (listen(server_sockfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("listening on port %d\n", server_port);

    while (1) {
        printf("wait\n");
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);
        struct thread_arg *arg = (struct thread_arg *)malloc(sizeof(struct thread_arg));
        arg->sock = client_sockfd;
        arg->addr = client_address;
        // TODO: This code must call pthread_join or pthread_detach. Current impl make memory leak.
        if (pthread_create(&(arg->self), NULL, &thread_main, (void *)arg) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
