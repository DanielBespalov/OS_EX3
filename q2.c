#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// A structure to represent an adjacency list node
typedef struct List {
    int dst;
    struct List* next;
} List;

// Function to create a new adjacency list node
List* newAdjListNode(int dst) {
    List* newNode = (List*)malloc(sizeof(List));
    newNode->dst = dst;
    newNode->next = NULL;
    return newNode;
}

// Function to add an edge to an adjacency list
void addEdge(List** graph, int src, int dst) {
    List* newNode = newAdjListNode(dst);
    newNode->next = graph[src];
    graph[src] = newNode;
}

// Function to create a reversed graph
List** getTranspose(List** graph, int V) {
    List** reverseGraph = (List**)malloc(V * sizeof(List*));
    for (int i = 0; i < V; i++) {
        reverseGraph[i] = NULL;
    }

    for (int v = 0; v < V; v++) {
        List* temp = graph[v];
        while (temp) {
            addEdge(reverseGraph, temp->dst, v);
            temp = temp->next;
        }
    }

    return reverseGraph;
}

// A utility function to perform DFS and fill the stack with vertices in the order of finishing times
void DFSUtil(List** graph, int v, int* visited, int* stack, int* stackIndex) {
    visited[v] = 1;
    List* temp = graph[v];
    while (temp) {
        if (!visited[temp->dst]) {
            DFSUtil(graph, temp->dst, visited, stack, stackIndex);
        }
        temp = temp->next;
    }
    stack[(*stackIndex)++] = v;
}

// A utility function to print the SCCs in the graph
void printSCCUtil(List** graph, int v, int* visited) {
    printf("%d ", v);
    visited[v] = 1;
    List* temp = graph[v];
    while (temp) {
        if (!visited[temp->dst]) {
            printSCCUtil(graph, temp->dst, visited);
        }
        temp = temp->next;
    }
}

int main() {
    int vert = 0, arcs = 0;
    printf("Enter the number of vertices and edges: ");
    scanf("%d %d", &vert, &arcs);

    List** graph = (List**)malloc(vert * sizeof(List*));
    for (int i = 0; i < vert; i++) {
        graph[i] = NULL;
    }

    srand(time(NULL)); // Seed the random number generator
    for (int i = 0; i < arcs; i++) {
        int src = rand() % vert; // Ensure src is within valid range
        int dst = rand() % vert; // Ensure dst is within valid range
        addEdge(graph, src, dst);
    }

    int* stack = (int*)malloc(vert * sizeof(int));
    int stackIndex = 0;
    int* visited = (int*)calloc(vert, sizeof(int));

    // Perform first pass of DFS and fill stack with vertices in the order of their finishing times
    for (int i = 0; i < vert; i++) {
        if (!visited[i]) {
            DFSUtil(graph, i, visited, stack, &stackIndex);
        }
    }

    // Create the reversed graph
    List** reverseGraph = getTranspose(graph, vert);

    memset(visited, 0, vert * sizeof(int));

    printf("Strongly Connected Components:\n");
    while (stackIndex > 0) {
        int v = stack[--stackIndex];
        if (!visited[v]) {
            printSCCUtil(reverseGraph, v, visited);
            printf("\n");
        }
    }

    // Free allocated memory
    for (int i = 0; i < vert; i++) {
        List* temp = graph[i];
        while (temp) {
            List* next = temp->next;
            free(temp);
            temp = next;
        }
    }
    free(graph);

    for (int i = 0; i < vert; i++) {
        List* temp = reverseGraph[i];
        while (temp) {
            List* next = temp->next;
            free(temp);
            temp = next;
        }
    }
    free(reverseGraph);
    free(stack);
    free(visited);

    return 0;
}
