#include "queue.h"
#include <stdlib.h>

queue_t *init(int queue_max_length, char *schedalg)
{

    queue_t *this = malloc(sizeof(queue_t));
    if (this == NULL)
    {
        return NULL;
    }
    this->queue_max_length = queue_max_length;
    this->length = 0;
    this->next_cell = 0;
    this->oldest_record = -1;
    this->working_threds = 0;
    this->schedalg = schedalg;
    qnode_t *array = malloc(sizeof(qnode_t) * queue_max_length);
    if (array == NULL)
    {
        free(this);
        return NULL;
    }
    this->queue = array;

    if (pthread_mutex_init(&this->lock, NULL) < 0)
    {
        free(array);
        free(this);
        return NULL;
    }
    if ((pthread_cond_init(&this->full, NULL) < 0) || (pthread_cond_init(&this->empty, NULL) < 0))
    {
        pthread_cond_destroy(&this->full);
        pthread_cond_destroy(&this->empty);
        pthread_mutex_destroy(&this->lock);
        free(array);
        free(this);
        return NULL;
    }
    return this;
}

void destroy(queue_t *q)
{
    if (q == NULL)
    {
        return;
    }
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->full);
    pthread_cond_destroy(&q->empty);
    free(q->queue);
    free(q);
}

int enqueue(queue_t *q, qnode_t node)
{
    if (q == NULL)
    {
        return -1;
    }
    pthread_mutex_lock(&q->lock);
    while ((q->length + q->working_threds) >= q->queue_max_length) // queue is full
    {
        if (!strcmp(q->schedalg, "block"))
        {
            // printf("queue is full 'Blocking', %d request %d threads\n", q->length, q->working_threds);
            pthread_cond_wait(&q->full, &q->lock);
        }
        else if ((q->length == 0) || (!strcmp(q->schedalg, "dt")))
        {
            // printf("queue is full 'Closing New Request', %d request %d threads\n", q->length, q->working_threds);
            Close(node.connfd);
            pthread_mutex_unlock(&q->lock);
            return 0;
        }
        else if (!strcmp(q->schedalg, "dh"))
        {
            qnode_t tmp;
            dequeue_unsafe(q, &tmp);
            Close(tmp.connfd);
        }
        else if (!strcmp(q->schedalg, "random"))
        {
            // printf("queue is full 'Droping Random', %d request %d threads\n", q->length, q->working_threds);
            drop_random(q);
        }
        else
        {
            // printf("queue is full 'Blocking', %d request %d threads\n", q->length, q->working_threds);
            pthread_cond_wait(&q->full, &q->lock);
        }
    }

    q->queue[q->next_cell] = node;
    if (q->length == 0)
    {
        q->oldest_record = q->next_cell;
    }
    q->next_cell = (q->next_cell + 1) % q->queue_max_length;
    q->length++;
    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int dequeue_unsafe(queue_t *q, qnode_t *node)
{
    if ((q == NULL) || (node == NULL))
    {
        return -1;
    }
    while (q->length <= 0)
    {
        pthread_cond_wait(&q->empty, &q->lock);
    }
    q->length--;
    *node = q->queue[q->oldest_record];
    q->oldest_record = (q->oldest_record + 1) % q->queue_max_length;
    if (q->length == 0)
    {
        q->oldest_record = -1;
    }
    return 0;
}

int dequeue(queue_t *q, qnode_t *node)
{

    if ((q == NULL) || (node == NULL))
    {
        return -1;
    }
    pthread_mutex_lock(&q->lock);
    dequeue_unsafe(q, node);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int handle(queue_t *q, qnode_t *node)
{
    if ((q == NULL) || (node == NULL))
    {
        return -1;
    }
    if (dequeue(q, node) == 0)
    {
        pthread_mutex_lock(&q->lock);
        q->working_threds++;
        pthread_mutex_unlock(&q->lock);
        return 0;
    }
    else
    {
        return -1;
    }
}

int done(queue_t *q)
{
    if (q == NULL)
    {
        return -1;
    }
    pthread_mutex_lock(&q->lock);
    q->working_threds--;
    pthread_cond_signal(&q->full);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int count_free_cells(queue_t *q)
{
    if (q == NULL)
    {
        return -1;
    }
    return q->queue_max_length - q->length;
}

int *_random_sub_set(int range)
{
    int *array = malloc(sizeof(int) * range);
    if (array == NULL)
    {
        return NULL;
    }
    int *sorted_array = malloc(sizeof(int) * range);
    if (sorted_array == NULL)
    {
        free(array);
        return NULL;
    }
    for (int i = 0; i < range; i++)
    {
        array[i] = i;
        sorted_array[i] = 0;
    }

    srand(time(NULL));
    int r;
    for (int len = (range - 1); len > 0; len--) // shufel array
    {
        r = rand() % (len + 1);
        int tmp = array[r];
        array[r] = array[len];
        array[len] = tmp;
    }

    // takes the first range/2 random indexs

    for (int i = 0; i < range / 2; i++)
    {
        sorted_array[array[i]] = 1;
    }
    free(array);
    return sorted_array;
}

void drop_random(queue_t *q) // asume that lock is locked
{
    if (q == NULL)
    {
        return;
    }
    qnode_t *new_array = malloc(sizeof(qnode_t) * q->queue_max_length);
    if (new_array == NULL)
    {
        exit(0);
    }
    int *random_array = _random_sub_set(q->length);
    int counter = 0;
    for (int i = 0; i < q->length; i++)
    {
        int index = (q->oldest_record + i) % q->queue_max_length;

        if (random_array[i])
        {
            new_array[counter] = q->queue[index];
            counter++;
        }
        else
        {
            Close(q->queue[index].connfd);
        }
    }
    if (counter == 0)
    {
        q->oldest_record = -1;
        q->next_cell = 0;
        q->length = 0;
    }
    else
    {
        q->oldest_record = 0;
        q->next_cell = counter;
        q->length = counter;
    }
    free(q->queue);
    q->queue = new_array;
    return;
}