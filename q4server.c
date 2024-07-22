#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "q3.h"

#define PORT 9034
#define BACKLOG 10 // How many pending connections queue will hold
#define BUFFER_SIZE 1024


void handle_client(int client_socket) {
    Graph* graph = NULL;

    char buffer[BUFFER_SIZE];
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);

        recv_msg(buffer,client_socket);

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

    }

    close(client_socket);
    printf("Client disconnected\n");
}

int main() {
    int server_fd, new_fd;
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
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
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
        new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), s, sizeof s);
        printf("Server: got connection from %s\n", s);

        if (!fork()) { // Child process
            close(server_fd);
            handle_client(new_fd);
            exit(0);
        }

        close(new_fd); // Parent process does not need this
    

    }
    
    return 0;

}
