#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9034
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

void recv_msg(char* buffer, int sockfd){
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if(bytes_received < 0) {
        perror("failed to recv");
        exit(1);
    } 
    else if(bytes_received == 0) 
    {
            printf("disconnected\n");
            exit(0);
    }

    printf("%s\n", buffer);
}

void send_msg(char* buffer, int sockfd)
{
    memset(buffer, 0, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);
        
    if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    int m;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    int yes = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr));
    memset(&(server_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    printf("Connected to server\n");


    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        printf("Menu:\n1. Call new graph-build a graph\n2. Calculate Kosraju on the graph\n3. Add new edge(enter i and j too)\n4. Remove edge\n");
        send_msg(buffer, sockfd);

        switch (buffer[0])
        {
        case '1':
            recv_msg(buffer, sockfd);
            send_msg(buffer, sockfd);
            m = (int)(buffer[2] - '0');
            recv_msg(buffer, sockfd);
            
            for(int i = 0; i< m; i++){
                send_msg(buffer, sockfd);
            } 
        break;
        
        case '2':
            recv_msg(buffer, sockfd);
        
            break;

        case '3':
            break;

        case '4':
            recv(sockfd, buffer, BUFFER_SIZE, 0);
            printf("%s", buffer);
            send_msg(buffer, sockfd);

            break;
        
        default:
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);

    }

    close(sockfd);
    return 0;
}
