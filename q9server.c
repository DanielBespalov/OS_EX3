#include "reactor.h"
#include "q3.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 9034
#define BACKLOG 10
#define BUFFER_SIZE 1024

pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;
Graph *graph = NULL;

void *handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        switch (buffer[0]) {
            case '1':
                pthread_mutex_lock(&graph_mutex);
                graph = newGraph(client_socket);
                pthread_mutex_unlock(&graph_mutex);
                break;
            case '2':
                pthread_mutex_lock(&graph_mutex);
                kosaraju(graph, client_socket);
                pthread_mutex_unlock(&graph_mutex);
                break;
            case '3':
                pthread_mutex_lock(&graph_mutex);
                graph = copyGraph(graph, buffer[2] - '0', buffer[4] - '0');
                pthread_mutex_unlock(&graph_mutex);
                break;
            case '4':
                pthread_mutex_lock(&graph_mutex);
                removeEdge(graph, client_socket);
                pthread_mutex_unlock(&graph_mutex);
                break;
            default:
                break;
        }
    }

    close(client_socket);
    printf("Client disconnected\n");
    return NULL;
}

void *handle_client_wrapper(int client_socket) {
    return handle_client(client_socket);
}

int main() {
    int server_fd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    memset(&(server_addr.sin_zero), '\0', 8);

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        sin_size = sizeof(client_addr);
        new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), s, sizeof s);
        printf("Server: got connection from %s\n", s);

        startProactor(new_fd, handle_client_wrapper);
    }

    close(server_fd);
    return 0;
}


