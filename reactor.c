#include "reactor.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int sockfd;
    proactorFunc func;
} ProactorArgs;

void* proactorThread(void* arg) {
    ProactorArgs* args = (ProactorArgs*)arg;
    args->func(args->sockfd);
    free(args);
    return NULL;
}

pthread_t startProactor(int sockfd, proactorFunc threadFunc) {
    pthread_t tid;
    ProactorArgs* args = (ProactorArgs*)malloc(sizeof(ProactorArgs));
    if (!args) {
        perror("malloc");
        exit(1);
    }
    args->sockfd = sockfd;
    args->func = threadFunc;

    if (pthread_create(&tid, NULL, proactorThread, args) != 0) {
        perror("pthread_create");
        free(args);
        exit(1);
    }
    return tid;
}

int stopProactor(pthread_t tid) {
    return pthread_cancel(tid);
}

void *reactorThread(void *arg) {
    Reactor *reactor = (Reactor *)arg;
    while (reactor->running) {
        fd_set read_fds = reactor->fds;
        int activity = select(reactor->max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            return NULL;
        }
        for (int i = 0; i <= reactor->max_fd; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                reactor->func[i](i);
            }
        }
    }
    return NULL;
}

void *startReactor() {
    Reactor *reactor = (Reactor *)malloc(sizeof(Reactor));
    if (!reactor) {
        perror("malloc");
        exit(1);
    }
    FD_ZERO(&reactor->fds);
    reactor->max_fd = 0;
    reactor->running = 1;
    if (pthread_create(&reactor->thread, NULL, reactorThread, reactor) != 0) {
        perror("pthread_create");
        exit(1);
    }
    return reactor;
}

int addFdToReactor(void *reactorPtr, int fd, reactorFunc func) {
    Reactor *reactor = (Reactor *)reactorPtr;
    if (fd >= FD_SETSIZE) {
        return -1;
    }
    FD_SET(fd, &reactor->fds);
    if (fd > reactor->max_fd) {
        reactor->max_fd = fd;
    }
    reactor->func[fd] = func;
    return 0;
}

int removeFdFromReactor(void *reactorPtr, int fd) {
    Reactor *reactor = (Reactor *)reactorPtr;
    if (fd >= FD_SETSIZE) {
        return -1;
    }
    FD_CLR(fd, &reactor->fds);
    if (fd == reactor->max_fd) {
        while (!FD_ISSET(reactor->max_fd, &reactor->fds) && reactor->max_fd > 0) {
            reactor->max_fd--;
        }
    }
    return 0;
}

int stopReactor(void *reactorPtr) {
    Reactor *reactor = (Reactor *)reactorPtr;
    reactor->running = 0;
    
    if (pthread_join(reactor->thread, NULL) != 0) {
        perror("pthread_join");
        return -1;
    }
    free(reactor);
    return 0;
}
