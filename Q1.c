#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void DFS(int v, int** graph, int* visited, int vert, int arcs, int* stack, int* stackIndex) {
    visited[v] = 1;
    for (int i = 0; i < arcs; i++) {
        if (graph[i][0] == v && !visited[graph[i][1]]) {
            DFS(graph[i][1], graph, visited, vert, arcs, stack, stackIndex);
        }
    }
    stack[(*stackIndex)++] = v;
}

void reverseGraph(int** graph, int** reversedGraph, int arcs) {
    for (int i = 0; i < arcs; i++) {
        reversedGraph[i][0] = graph[i][1];
        reversedGraph[i][1] = graph[i][0];
    }
}

void printSCCs(int v, int** reversedGraph, int* visited, int vert, int arcs) {
    printf("%d ", v);
    visited[v] = 1;
    for (int i = 0; i < arcs; i++) {
        if (reversedGraph[i][0] == v && !visited[reversedGraph[i][1]]) {
            printSCCs(reversedGraph[i][1], reversedGraph, visited, vert, arcs);
        }
    }
}

int main() {
    int vert = 0, arcs = 0;
    printf("\nWrite please num of verts and after num of arcs: \n");
    scanf("%d %d", &vert, &arcs);

    int** graph = (int**)malloc(sizeof(int*) * arcs);

    for (int i = 0; i < arcs; i++) {
        graph[i] = (int*)malloc(sizeof(int) * 2);
        graph[i][0] = rand() % vert; 
        graph[i][1] = rand() % vert;
    }

    int* stack = (int*)malloc(sizeof(int) * vert);
    int stackIndex = 0;
    int* visited = (int*)calloc(vert, sizeof(int));

    for (int i = 0; i < vert; i++) {
        if (!visited[i]) {
            DFS(i, graph, visited, vert, arcs, stack, &stackIndex);
        }
    }

    int** reversedGraph = (int**)malloc(sizeof(int*) * arcs);
    for (int i = 0; i < arcs; i++) {
        reversedGraph[i] = (int*)malloc(sizeof(int) * 2);
    }
    reverseGraph(graph, reversedGraph, arcs);

    memset(visited, 0, vert * sizeof(int));

    printf("Strongly Connected Components:\n");
    while (stackIndex > 0) {
        int v = stack[--stackIndex];
        if (!visited[v]) {
            printSCCs(v, reversedGraph, visited, vert, arcs);
            printf("\n");
        }
    }

    for (int i = 0; i < arcs; i++) {
        free(graph[i]);
        free(reversedGraph[i]);
    }
    free(graph);
    free(reversedGraph);
    free(stack);
    free(visited);

    return 0;
}
