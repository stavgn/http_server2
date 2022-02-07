#ifndef __STATS_H__
#define __STATS_H__
#include <pthread.h>
#include "queue.h"

typedef struct StatsManager
{
    int n_ack_threads;
    int n_threads;
    pthread_t *ack_threads;
    int *per_thread_static_requests_counter;
    int *per_thread_total_requests_counter;
    int *per_thread_dynamic_requests_counter;
    qnode_t *per_thread_requests;
    pthread_mutex_t lock;

} sttmngr_t;

sttmngr_t *statManager;

void init_array(int *arr, int size);
int find_slot();

void init_stat(int nthreads);
void inc_static();
void inc_total();
void acknowledge_thread();
void destroy_stat();
void write_header(char *hdr, char *buf);
void write_all_headers(char *buf);
void load_request(qnode_t req);
void inc_dynamic();

#endif
