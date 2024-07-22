#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "q3.h"

#define PORT 9034
#define BACKLOG 10 // How many pending connections queue will hold
#define BUFFER_SIZE 1024

pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;
Graph* graph = NULL;

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv_msg(buffer, client_socket);

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
                graph = copyGraph(graph, (int)(buffer[2] - '0'), (int)(buffer[4] - '0'));
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
    pthread_exit(NULL);
}

void *client_thread(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    handle_client(client_socket);
    return NULL;
}

int main() {
    int server_fd, *new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    memset(&(server_addr.sin_zero), '\0', 8);

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }

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

    while (1) {
        sin_size = sizeof(client_addr);
        new_fd = malloc(sizeof(int));
        if (!new_fd) {
            perror("malloc");
            continue;
        }
        *new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (*new_fd == -1) {
            perror("accept");
            free(new_fd);
            continue;
        }

        inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), s, sizeof s);
        printf("Server: got connection from %s\n", s);

        pthread_t thread;
        if (pthread_create(&thread, NULL, client_thread, new_fd) != 0) {
            perror("pthread_create");
            free(new_fd);
            continue;
        }

        pthread_detach(thread); // So that we don't need to join the threads
    }

    return 0;
}
