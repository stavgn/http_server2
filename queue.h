#ifndef __QUEUE__
#define __QUEUE__
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include "segel.h"
typedef struct QueueNode
{
    pthread_t thread_id;
    int connfd;
    struct timeval createdAt;
    struct timeval handledAt;

} qnode_t;

typedef struct Queue
{
    int queue_max_length;
    int length;
    int next_cell;
    int oldest_record;
    int working_threds;
    char *schedalg;
    qnode_t *queue;
    pthread_mutex_t lock;
    pthread_cond_t full;
    pthread_cond_t empty;

} queue_t;

queue_t *init(int queue_max_length, char *schedalg);
int enqueue(queue_t *q, qnode_t node);
int dequeue(queue_t *q, qnode_t *node);
int dequeue_unsafe(queue_t *q, qnode_t *node);
int handle(queue_t *q, qnode_t *node);
int done(queue_t *q);
int count_free_cells(queue_t *q);
void destroy(queue_t *q);
int *_random_sub_set(int range);
void drop_random(queue_t *q);

#endif
