/*
 * Mazart - Deque
 *  Module provides a basic queue+stack structure.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#ifndef _DEQUE_H_
#define _DEQUE_H_

#include "common.h"

/*
 * Deque Struct
 *  Provides a simple interface for most queue and stack based operations.
 *  Can store any object pointer without transfer of ownership.  NULL
 *  pointers are allowed.
 */
typedef struct deque_st deque_t;

/* - - Deque API - - */

/* Deque constructor and destructor. */
deque_t *CreateDeque(void);
void FreeDeque(deque_t *deque);

/* Insert an element.  Returns true upon success. */
bool_t PushDequeFirst(deque_t *deque, void *item);
bool_t PushDequeLast(deque_t *deque, void *item);
/* Get element, but don't remove it. */
void *PeekDequeFirst(deque_t const *deque);
void *PeekDequeLast(deque_t const *deque);
/* Get and remove element. */
void *PopDequeFirst(deque_t *deque);
void *PopDequeLast(deque_t *deque);

/* Number of elements currently stored in the Deque. */
size_t DequeSize(deque_t const *deque);

/* Removes all elements, no action is made on the pointer to object. */
void ClearDeque(deque_t *deque);
/* Removes all elements, calling free() on any non-null element. */
void ClearDequeFreeItems(deque_t *deque);
/* Removes all elements, calling the provided dtor() on any non-null
 * element.  The destructor must be non-NULL. */
void ClearDequeDestroyItems(deque_t *deque, void (*dtor)(void *));

#endif /* _DEQUE_H_ */
