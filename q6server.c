#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "reactor.h"
#include "q3.h"

#define PORT 9034
#define BACKLOG 10
#define BUFFER_SIZE 1024

Graph* graph = NULL;
void handle_client(int client_socket);

void client_handler(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';

        switch (buffer[0])
        {
        case '1':
            graph = newGraph(client_socket);
            break;
        
        case '2':
            kosaraju(graph, client_socket);
            break;

        case '3':
            graph = copyGraph(graph, (int)(buffer[2] - '0'), (int)(buffer[4] - '0'));
            break;

        case '4':
            removeEdge(graph, client_socket);
            break;
        
        default:
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);

    }

    if (bytes_received == 0) {
        printf("Client %d disconnected\n", client_socket);
    } else if (bytes_received < 0) {
        perror("recv");
    }

    close(client_socket);
}

void accept_handler(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(client_addr);
    int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
        return;
    }

    printf("Server: got connection from %s\n", inet_ntoa(client_addr.sin_addr));
    addFdToReactor(startReactor(), new_fd, client_handler);
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    Reactor *reactor = startReactor();
    addFdToReactor(reactor, server_fd, accept_handler);

    // Main loop
    while (1) {
        sleep(1); // Just to keep the main thread running
    }

    stopReactor(reactor);
    close(server_fd);
    return 0;
}
