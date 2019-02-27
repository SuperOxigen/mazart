/*
 * Mazart - Priority Queue
 *  Module provides a Priority Queue data structure.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _PRIORITY_H_
#define _PRIORITY_H_

#include "common.h"

/*
 * Priority Queue Struct
 *  A handle to a basic priority queue.  Provides an enqueue, peek and
 *  pop function. Provides a clearing API similar to other data
 *  structures in the project.
 */
typedef struct priority_queue_st priority_queue_t;

/* - - Priority Queue - - */
/* Priority Queue constructor.  Creates and empty Priority Queue. */
priority_queue_t *CreatePriorityQueue(void);
/* Priority Queue destructor.  Frees Priority Queue resources.
 * Elements that need to be freed must be free in a separate call to
 * ClearPriorityQueueFreeItems() or ClearPriorityQueueDestroyItems()
 * before calling this destructor. */
void FreePriorityQueue(priority_queue_t *queue);

/* Enqueues and item.  The larger the priority value, the higher the
 * priority (max-heap). */
bool_t EnqueuePriority(priority_queue_t *queue, size_t priority, void *item);
/* Get the highest priority item without removing it. */
void *PeekTopPriority(priority_queue_t const *queue);
/* Get the highest priority item and remove it. */
void *PopTopPriority(priority_queue_t *queue);

/* Number of elements currently stored in the Priority Queue. */
size_t PriorityQueueSize(priority_queue_t const *queue);

/* Clears the Priority Queue items, without freeing the store items. */
void ClearPriorityQueue(priority_queue_t *queue);
/* Clears the Priority Queue items, calling free() on non-null items.  */
void ClearPriorityQueueFreeItems(priority_queue_t *queue);
/* Clears the Priority Queue items, calling the provided dtor() on
 * non-null items.  The provided dtor must not be NULL. */
void ClearPriorityQueueDestroyItems(priority_queue_t *queue, void (*dtor)(void *));

#endif /* _PRIORITY_H_ */
