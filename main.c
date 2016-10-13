#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "threadpool.h"
#include "list.h"
#include "mergesort.h"

#define USAGE "usage: ./sort [thread_count] [input_count]\n"

#ifdef BENCH
#define CLOCK_ID CLOCK_MONOTONIC_RAW
#endif

struct {
    pthread_mutex_t mutex;
    int cut_thread_count;
} data_context;

llist_t *tmp_list;
llist_t *the_list = NULL;

int thread_count = 0, data_count = 0, max_cut = 0;
tpool_t *pool = NULL;

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

static void *task_run(void *data)
{
    (void) data;
    while (1) {
        task_t *_task = tqueue_pop(pool->queue);
        if (_task) {
            if (!_task->func) {
                tqueue_push(pool->queue, _task);
                break;
            } else {
                _task->func(_task->arg);
                free(_task);
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    if (argc < 3) {
        printf(USAGE);
        return -1;
    }
#ifdef BENCH
    struct timespec start, end;
#endif
    thread_count = atoi(argv[1]);
    data_count = atoi(argv[2]);
    max_cut = thread_count * (thread_count <= data_count) +
              data_count * (thread_count > data_count) - 1;

    /* Read data */
    the_list = list_new();

#ifndef BENCH
    printf("input unsorted data line-by-line\n");
#endif
    for (int i = 0; i < data_count; ++i) {
        long int data;
        scanf("%ld", &data);
        list_add(the_list, data);
    }

    /* initialize tasks inside thread pool */
    pthread_mutex_init(&(data_context.mutex), NULL);
    data_context.cut_thread_count = 0;
    tmp_list = NULL;
    pool = (tpool_t *) malloc(sizeof(tpool_t));
    tpool_init(pool, thread_count, task_run);

#ifdef BENCH
    clock_gettime(CLOCK_ID, &start);
#endif
    /* launch the first task */
    task_t *_task = (task_t *) malloc(sizeof(task_t));
    _task->func = cut_func;
    _task->arg = the_list;
    tqueue_push(pool->queue, _task);
    /* close thread pool */
    tpool_close(pool);
#ifdef BENCH
    clock_gettime(CLOCK_ID, &end);
    printf("%lf,", diff_in_second(start, end));
#endif
    /* release thread pool */
    tpool_free(pool);

    return 0;
}
