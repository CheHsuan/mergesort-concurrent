#include "mergesort.h"

extern struct {
    pthread_mutex_t mutex;
    int cut_thread_count;
} data_context;

extern llist_t *tmp_list;
extern llist_t *the_list;

extern int thread_count, data_count, max_cut;
extern tpool_t *pool;

llist_t *merge_list(llist_t *a, llist_t *b)
{
    llist_t *_list = list_new();
    node_t *current = NULL;
    while (a->size && b->size) {
        llist_t *small = (llist_t *)
                         ((intptr_t) a * (a->head->data <= b->head->data) +
                          (intptr_t) b * (a->head->data > b->head->data));
        if (current) {
            current->next = small->head;
            current = current->next;
        } else {
            _list->head = small->head;
            current = _list->head;
        }
        small->head = small->head->next;
        --small->size;
        ++_list->size;
        current->next = NULL;
    }

    llist_t *remaining = (llist_t *) ((intptr_t) a * (a->size > 0) +
                                      (intptr_t) b * (b->size > 0));
    if (current) current->next = remaining->head;
    _list->size += remaining->size;
    free(a);
    free(b);
    return _list;
}

llist_t *merge_sort(llist_t *list)
{
    if (list->size < 2)
        return list;
    int mid = list->size / 2;
    llist_t *left = list;
    llist_t *right = list_new();
    right->head = list_nth(list, mid);
    right->size = list->size - mid;
    list_nth(list, mid - 1)->next = NULL;
    left->size = mid;
    return merge_list(merge_sort(left), merge_sort(right));
}

void merge(void *data)
{
    llist_t *_list = (llist_t *) data;
    if (_list->size < (uint32_t) data_count) {
        pthread_mutex_lock(&(data_context.mutex));
        llist_t *_t = tmp_list;
        if (!_t) {
            tmp_list = _list;
            pthread_mutex_unlock(&(data_context.mutex));
        } else {
            tmp_list = NULL;
            pthread_mutex_unlock(&(data_context.mutex));
            task_t *_task = (task_t *) malloc(sizeof(task_t));
            _task->func = merge;
            _task->arg = merge_list(_list, _t);
            tqueue_push(pool->queue, _task);
        }
    } else {
        the_list = _list;
        task_t *_task = (task_t *) malloc(sizeof(task_t));
        _task->func = NULL;
        tqueue_push(pool->queue, _task);
#ifndef AUTOTEST
        list_print(_list);
#endif
    }
}

void cut_func(void *data)
{
    llist_t *list = (llist_t *) data;
    pthread_mutex_lock(&(data_context.mutex));
    int cut_count = data_context.cut_thread_count;
    if (list->size > 1 && cut_count < max_cut) {
        ++data_context.cut_thread_count;
        pthread_mutex_unlock(&(data_context.mutex));

        /* cut list */
        int mid = list->size / 2;
        llist_t *_list = list_new();
        _list->head = list_nth(list, mid);
        _list->size = list->size - mid;
        list_nth(list, mid - 1)->next = NULL;
        list->size = mid;

        /* create new task: left */
        task_t *_task = (task_t *) malloc(sizeof(task_t));
        _task->func = cut_func;
        _task->arg = _list;
        tqueue_push(pool->queue, _task);

        /* create new task: right */
        _task = (task_t *) malloc(sizeof(task_t));
        _task->func = cut_func;
        _task->arg = list;
        tqueue_push(pool->queue, _task);
    } else {
        pthread_mutex_unlock(&(data_context.mutex));
        merge(merge_sort(list));
    }
}
