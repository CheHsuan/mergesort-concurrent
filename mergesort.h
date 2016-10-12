#include <pthread.h>
#include <stdlib.h>

#include "threadpool.h"
#include "list.h"

llist_t *merge_list(llist_t *a, llist_t *b);
llist_t *merge_sort(llist_t *list);
void merge(void *data);
void cut_func(void *data);
