#ifndef _PRIORITY_H_
#define _PRIORITY_H_

#include "common.h"

typedef struct priority_queue_st priority_queue_t;

priority_queue_t *CreatePriorityQueue(void);
void FreePriorityQueue(priority_queue_t *queue);

bool_t EnqueuePriority(priority_queue_t *queue, size_t priority, void *item);
void *PeekTopPriority(priority_queue_t const *queue);
void *PopTopPriority(priority_queue_t *queue);

size_t PriorityQueueSize(priority_queue_t const *queue);

void ClearPriorityQueue(priority_queue_t *queue);
void ClearPriorityQueueFreeItems(priority_queue_t *queue);
void ClearPriorityQueueDestroyItems(priority_queue_t *queue, void (*dtor)(void *));

#endif /* _PRIORITY_H_ */
