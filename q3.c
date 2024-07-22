#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "q3.h"
#include "reactor.h"

#define BUFFER_SIZE 1024

pthread_mutex_t scc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t scc_cond = PTHREAD_COND_INITIALIZER;
int largest_scc_size = 0;
int scc_update_flag = 0;
int total_vertices = 0;
int first_scc_printed = 0;  // To track the first print
int current_scc_state = 0;  // 0: No SCC > 50%, 1: SCC > 50%, 2: SCC > 50% but then < 50%

void recv_msg(char* buffer, int sockfd) {
    memset(buffer, 0, BUFFER_SIZE);

    if (sockfd != 0) {  
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected\n");
            } else {
                perror("recv");
            }
            close(sockfd);
            pthread_exit(NULL); // Exit the thread if the client disconnects
        }
    } else {
        fgets(buffer, 4, stdin);
    }
}

void send_msg(char* buffer, int sockfd) {   
    if (sockfd != 0) {     
        if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            close(sockfd);
            exit(1);
        }
    } else {
        printf("%s\n", buffer);
    }
}

// Function to create a new adjacency list node
List* newAdjListNode(int dst) {
    List* newNode = (List*)malloc(sizeof(List));
    newNode->dst = dst;
    newNode->next = NULL;
    return newNode;
}

// Function to copy a graph represented as an adjacency list
Graph* copyGraph(Graph* srcGraph, int src, int dst) {
    if (!srcGraph) return NULL;

    // Create a new graph
    Graph* newGraph = (Graph*)malloc(sizeof(Graph));
    if (!newGraph) {
        perror("malloc");
        exit(1);
    }

    newGraph->V = (srcGraph->V) + 1;
    newGraph->m = (srcGraph->m) + 1;
    newGraph->adjList = (List**)calloc(newGraph->V, sizeof(List*));
    if (!newGraph->adjList) {
        perror("calloc");
        exit(1);
    }

    // Copy all adjacency lists
    for (int i = 0; i < srcGraph->V; i++) {
        List* current = srcGraph->adjList[i];
        List* newHead = NULL;
        List* newTail = NULL;

        while (current) {
            // Create a new node
            List* newNode = (List*)malloc(sizeof(List));
            if (!newNode) {
                perror("malloc");
                exit(1);
            }
            newNode->dst = current->dst;
            newNode->next = NULL;

            // Append the new node to the new list
            if (!newHead) {
                newHead = newNode;
                newTail = newNode;
            } else {
                newTail->next = newNode;
                newTail = newNode;
            }

            // Move to the next node in the original list
            current = current->next;
        }

        // Attach the new list to the new graph
        newGraph->adjList[i] = newHead;
    }

    newEdge(newGraph, src, dst);

    return newGraph;
}

// Function to create a new graph with n vertices and m edges
Graph* newGraph(int client_socket) {
    int n = 0;
    char buffer[BUFFER_SIZE];
    
    send_msg("Enter number of vertices and edges: \n", client_socket);
    memset(buffer, 0, BUFFER_SIZE);

    recv_msg(buffer, client_socket);
    n = atoi(buffer);

    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) {
        perror("malloc");
        exit(1);
    }

    graph->V = n;
    graph->adjList = (List**)malloc(n * sizeof(List*));
    if (!graph->adjList) {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        graph->adjList[i] = NULL;
    }

    graph->m = atoi(buffer + 2);
    send_msg("Enter edges (src dst):\n", client_socket);
    
    for (int i = 0; i < graph->m; i++) {
        int src, dst;
        memset(buffer, 0, BUFFER_SIZE);
        recv_msg(buffer, client_socket);
        
        src = atoi(buffer);
        dst = atoi(buffer + 2); 
        newEdge(graph, src, dst);
    }
    return graph;
}

// Function to add an edge to an adjacency list
void newEdge(Graph* graph, int src, int dst) {
    List* newNode = newAdjListNode(dst);
    newNode->next = graph->adjList[src];
    graph->adjList[src] = newNode;
}

// Function to remove an edge from an adjacency list
void removeEdge(Graph* graph, int client_socket) {
    int src, dst;
    char buffer[BUFFER_SIZE];
    
    memset(buffer, 0, BUFFER_SIZE);
    send_msg("Enter src dst:\n", client_socket);
    recv_msg(buffer, client_socket);

    src = atoi(buffer);
    dst = atoi(buffer + 2);

    List* temp = graph->adjList[src];
    List* prev = NULL;

    // If the edge to be removed is the head node
    if (temp != NULL && temp->dst == dst) {
        graph->adjList[src] = temp->next;
        free(temp);
        return;
    }

    // Search for the edge to be removed, keeping track of the previous node
    while (temp != NULL && temp->dst != dst) {
        prev = temp;
        temp = temp->next;
    }

    // If the edge was not present in the adjacency list
    if (temp == NULL) return;

    // Unlink the node from the adjacency list
    prev->next = temp->next;
    free(temp);
    printf("Edge removed\n");
}

// Function to create a reversed graph
Graph* getTranspose(Graph* graph, int n) {
    Graph* reverseGraph = (Graph*)malloc(sizeof(Graph));
    if (!reverseGraph) {
        perror("calloc");
        exit(1);
    }

    reverseGraph->V = n;
    reverseGraph->adjList = (List**)malloc(n * sizeof(List*));
    for (int i = 0; i < n; i++) {
        reverseGraph->adjList[i] = NULL;
    }

    for (int v = 0; v < n; v++) {
        List* temp = graph->adjList[v];        
        while (temp) {
            newEdge(reverseGraph, temp->dst, v);
            temp = temp->next;
        }
    }

    return reverseGraph;
}

// A utility function to perform DFS and fill the stack with vertices in the order of finishing times
void DFSUtil(Graph* graph, int v, int* visited, int* stack, int* stackIndex) {
    if (v >= graph->V) return;
    visited[v] = 1;
    List* temp = graph->adjList[v];
    while (temp) {
        if (!visited[temp->dst]) {
            DFSUtil(graph, temp->dst, visited, stack, stackIndex);
        }
        temp = temp->next;
    }
    stack[(*stackIndex)++] = v;
}

// A utility function to print the SCCs in the graph
void printSCCUtil(Graph* graph, int v, int* visited, int client_socket) {
    if (v > graph->V)
        return;

    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%d ", v);
    send_msg(buffer, client_socket);

    visited[v] = 1;

    List* temp = graph->adjList[v];
    while (temp) {
        if (!visited[temp->dst]) {
            printSCCUtil(graph, temp->dst, visited, client_socket);
        }
        temp = temp->next;
    }
}



// Function to find and print strongly connected components using Kosaraju's algorithm
void kosaraju(Graph* graph, int client_socket) {
    int n = graph->V;
    total_vertices = n;  // Set total vertices

    int* stack = (int*)malloc(n * sizeof(int));
    if (!stack) {
        perror("malloc");
        exit(1);
    }

    int stackIndex = 0;
    int* visited = (int*)calloc(n, sizeof(int));
    if (!visited) {
        perror("calloc");
        exit(1);
    }

    // Perform first pass of DFS and fill stack with vertices in the order of their finishing times
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            DFSUtil(graph, i, visited, stack, &stackIndex);
        }
    }

    Graph* reverseGraph = getTranspose(graph, n);

    memset(visited, 0, n * sizeof(int));

    send_msg("Strongly Connected Components:\n", client_socket);

    int maxSCCSize = 0;
    int currentSCCSize = 0;

    while (stackIndex > 0) {
        int v = stack[--stackIndex];
        if (!visited[v]) {
            currentSCCSize = 0;
            printSCCUtil(reverseGraph, v, visited, client_socket);

            char buffer[BUFFER_SIZE];
            sprintf(buffer, "\n");
            send_msg(buffer, client_socket);

            if (currentSCCSize > maxSCCSize) {
                maxSCCSize = currentSCCSize;
            }
        }
    }

    free(reverseGraph);
    free(stack);
    free(visited);
}


