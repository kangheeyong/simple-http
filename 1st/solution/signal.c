#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

// 이 예제에서는 연결된 client가 하나니까 이렇게 관리하는 것.
int client_socket = -1;

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

void sig_handler(int sig) {
    struct linger linger;

    // Signal handling중에 다시 signal이 발생하면 어떻게 될까?
    printf("Signal %d received!\n", sig);
    if (sig != SIGINT) {
        return;
    }

    if (client_socket >= 0) {
        printf("close socket %d\n", client_socket);
        close(client_socket);
    }

    //이 함수 밖을 나가면 signal로 점프하기 직전 상태로 돌아간다.
    //exit(-1);
}

void install_signal_handler() {
    struct sigaction old, new;
    new.sa_handler = sig_handler;
    sigemptyset(&new.sa_mask);
    new.sa_flags = 0;
    sigaction(SIGINT, &new, &old);
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

    //signal hander설치
    install_signal_handler();

    while (1) {
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);
        client_socket = client_sockfd;
        // How do we fix it to support multiple connection?
        echo(client_sockfd, client_address);
    }
    return 0;
}
