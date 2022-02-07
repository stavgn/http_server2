#include <pthread.h>
#include "segel.h"
#include "request.h"
#include "queue.h"
#include "stats.h"
//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too

queue_t *incoming_requests;

void getargs(int *port, int *nthreads, int *queue_size, char **schedalg, int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, " Usage : %s <port><number of threads><queue size><schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *nthreads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    *schedalg = argv[4];
}

void *thread_worker()
{
    acknowledge_thread();
    while (1)
    {
        qnode_t request;
        int err = handle(incoming_requests, &request);
        if (err == -1)
        {
            printf("No request found");
            continue;
        }
        struct timeval now;
        gettimeofday(&now, NULL);
        timersub(&now, &request.createdAt, &request.handledAt);
        request.thread_id = pthread_self();
        load_request(request);
        requestHandle(request.connfd);
        Close(request.connfd);
        done(incoming_requests);
    }
    return NULL;
}

void init_threads(int nthreads)
{
    while (nthreads--)
    {
        pthread_t t;
        pthread_create(&t, NULL, thread_worker, NULL);
    }
};

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, nthreads, queue_size;
    char *schedalg;
    struct sockaddr_in clientaddr;

    getargs(&port, &nthreads, &queue_size, &schedalg, argc, argv);

    incoming_requests = init(queue_size, schedalg);
    init_stat(nthreads);

    init_threads(nthreads);
    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        qnode_t request;
        gettimeofday(&request.createdAt, NULL);
        request.connfd = connfd;
        enqueue(incoming_requests, request);
    }
}
