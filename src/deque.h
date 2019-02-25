#ifndef _DEQUE_H_
#define _DEQUE_H_

#include "common.h"

typedef struct deque_st deque_t;

deque_t *CreateDeque(void);
void FreeDeque(deque_t *deque);

bool_t Enqueue(deque_t *deque, void *item);
bool_t PushDeque(deque_t *deque, void *item);
void *PeekDeque(deque_t const *deque);
void *PopDeque(deque_t *deque);

size_t DequeSize(deque_t const *deque);

void ClearDeque(deque_t *deque);
void ClearDequeFreeItems(deque_t *deque);
void ClearDequeDestroyItems(deque_t *deque, void (*dtor)(void *));

#endif /* _DEQUE_H_ */
