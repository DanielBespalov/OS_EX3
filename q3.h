#ifndef q3_H
#define q3_H

// A structure to represent an adjacency list node
typedef struct List {
    int dst;
    struct List* next;
} List;

typedef struct Graph {
    int V;
    int m;
    List** adjList;
} Graph;


// Function prototypes
Graph* copyGraph(Graph* srcGraph, int src, int dst);
void recv_msg(char* buffer, int sockfd);
void send_msg(char* buffer, int sockfd);
List* newAdjListNode(int dst);
Graph* newGraph(int client_socket);
void newEdge(Graph* graph, int src, int dst);
void removeEdge(Graph*, int client_socket);
Graph* getTranspose(Graph*, int n);
void DFSUtil(Graph* graph, int v, int* visited, int* stack, int* stackIndex);
void printSCCUtil(Graph* graph, int v, int* visited, int client_socket);
void kosaraju(Graph* graph, int client_socket);

#endif
