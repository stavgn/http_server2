#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "stats.h"

void init_stat(int nthreads)
{
    statManager = malloc(sizeof(sttmngr_t));

    if (pthread_mutex_init(&statManager->lock, NULL) < 0)
    {
        return;
    }

    pthread_t *ack_threads = malloc(sizeof(pthread_t) * nthreads);
    int *per_thread_static_requests_counter = malloc(sizeof(int) * nthreads);
    int *per_thread_dynamic_requests_counter = malloc(sizeof(int) * nthreads);
    int *per_thread_total_requests_counter = malloc(sizeof(int) * nthreads);
    qnode_t *per_thread_requests = malloc(sizeof(qnode_t) * nthreads);

    init_array(per_thread_static_requests_counter, nthreads);
    init_array(per_thread_dynamic_requests_counter, nthreads);
    init_array(per_thread_total_requests_counter, nthreads);

    statManager->ack_threads = ack_threads;
    statManager->per_thread_static_requests_counter = per_thread_static_requests_counter;
    statManager->per_thread_dynamic_requests_counter = per_thread_dynamic_requests_counter;
    statManager->n_ack_threads = 0;
    statManager->per_thread_requests = per_thread_requests;
    statManager->per_thread_total_requests_counter = per_thread_total_requests_counter;
    statManager->n_threads = nthreads;
}

void acknowledge_thread()
{
    pthread_mutex_lock(&statManager->lock);
    pthread_t curr_thread = pthread_self();
    statManager->ack_threads[statManager->n_ack_threads] = curr_thread;
    statManager->n_ack_threads++;
    pthread_mutex_unlock(&statManager->lock);
}

void inc_static()
{
    int s = find_slot();
    if (s == -1)
    {
        return;
    }
    statManager->per_thread_static_requests_counter[s]++;
}

void inc_total()
{
    int s = find_slot();
    if (s == -1)
    {
        return;
    }
    statManager->per_thread_total_requests_counter[s]++;
}

void inc_dynamic()
{
    int s = find_slot();
    if (s == -1)
    {
        return;
    }
    statManager->per_thread_dynamic_requests_counter[s]++;
}

int find_slot()
{
    pthread_t curr_thread = pthread_self();
    for (int i = 0; i < statManager->n_ack_threads; i++)
    {
        if (statManager->ack_threads[i] == curr_thread)
        {
            return i;
        }
    }
    return -1;
}

void write_header(char *hdr, char *buf)
{
    int s = find_slot();
    if (s == -1)
    {
        return;
    }

    if (!strcmp(hdr, "Stat-Thread-Static"))
    {
        int val = statManager->per_thread_static_requests_counter[s];
        sprintf(buf, "%s%s:: %d\r\n", buf, hdr, val);
    }
    if (!strcmp(hdr, "Stat-Thread-Count"))
    {
        int val = statManager->per_thread_total_requests_counter[s];
        sprintf(buf, "%s%s:: %d\r\n", buf, hdr, val);
    }
    if (!strcmp(hdr, "Stat-Req-Arrival"))
    {
        struct timeval val = statManager->per_thread_requests[s].createdAt;
        sprintf(buf, "%s%s:: %ld.%06ld\r\n", buf, hdr, val.tv_sec, val.tv_usec);
    }
    if (!strcmp(hdr, "Stat-Req-Dispatch"))
    {
        struct timeval val = statManager->per_thread_requests[s].handledAt;
        sprintf(buf, "%s%s:: %ld.%06ld\r\n", buf, hdr, val.tv_sec, val.tv_usec);
    }
    if (!strcmp(hdr, "Stat-Thread-Id"))
    {
        int s = find_slot();
        sprintf(buf, "%s%s:: %d\r\n", buf, hdr, s);
    }
    if (!strcmp(hdr, "Stat-Thread-Dynamic"))
    {
        int val = statManager->per_thread_dynamic_requests_counter[s];
        sprintf(buf, "%s%s:: %d\r\n", buf, hdr, val);
    }
}

void init_array(int *arr, int size)
{
    for (int i = 0; i > size; i++)
    {
        arr[i] = 0;
    }
}

void load_request(qnode_t req)
{
    int s = find_slot();
    statManager->per_thread_requests[s] = req;
}

void destroy_stat()
{
    free(statManager->ack_threads);
    free(statManager->per_thread_dynamic_requests_counter);
    free(statManager->per_thread_static_requests_counter);
    free(statManager->per_thread_requests);
    free(statManager->per_thread_total_requests_counter);
    pthread_mutex_destroy(&statManager->lock);
    free(statManager);
}

void write_all_headers(char *buf)
{
    write_header("Stat-Req-Arrival", buf);
    write_header("Stat-Req-Dispatch", buf);
    write_header("Stat-Thread-Id", buf);
    write_header("Stat-Thread-Count", buf);
    write_header("Stat-Thread-Static", buf);
    write_header("Stat-Thread-Dynamic", buf);
    sprintf(buf, "%s\r\n", buf);
}