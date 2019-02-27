/*
 * Mazart - Deque
 *  Module provides a basic queue+stack structure.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include "deque.h"

#include <stdlib.h>
#include <string.h>

/* - - Deque Node Structure - - */
typedef struct deque_node_st deque_node_t;
struct deque_node_st {
  void *item;
  deque_node_t *preceding;  /* Closer to front */
  deque_node_t *succeeding;  /* Closer to back */
};

/* - - Deque Structure - - */
struct deque_st {
  size_t size;
  deque_node_t *first;
  deque_node_t *last;
};

/* - - Deque Node Internal API Prototypes - - */

/* Deque Node constructor - Allocates a new Deque Node on the heap. */
static deque_node_t *CreateDequeNode(void *item);
/* Deque Node destructor - Frees node resources, but does not effect the
 * object pointed to by the node. */
static void FreeDequeNode(deque_node_t *node);

/* - - Deque API - - */

deque_t *CreateDeque(void)
{
  deque_t *deque;
  deque = calloc(1, sizeof(deque_t));
  return deque;
}

void FreeDeque(deque_t *deque)
{
  if (!deque) return;
  ClearDeque(deque);
  free(deque);
}

/* - Push Operations - */

bool_t PushDequeFirst(deque_t *deque, void *item)
{
  deque_node_t *node;
  if (!deque) return false;
  node = CreateDequeNode(item);
  if (deque->first)
  {
    deque->first->preceding = node;
    node->succeeding = deque->first;
  }
  else /* No other element in Deque */
  {
    deque->last = node;
  }
  deque->first = node;
  deque->size++;
  return true;
}

bool_t PushDequeLast(deque_t *deque, void *item)
{
  deque_node_t *node;
  if (!deque) return false;
  node = CreateDequeNode(item);
  if (deque->last)
  {
    deque->last->succeeding = node;
    node->preceding = deque->last;
  }
  else /* No other element in Deque */
  {
    deque->first = node;
  }
  deque->last = node;
  deque->size++;
  return true;
}

/* - Peek Operations - */

void *PeekDequeFirst(deque_t const *deque)
{
  if (!deque || !deque->first) return NULL;
  return deque->first->item;
}

void *PeekDequeLast(deque_t const *deque)
{
  if (!deque || !deque->last) return NULL;
  return deque->last->item;
}

/* - Pop Operations - */

void *PopDequeFirst(deque_t *deque)
{
  void *item;
  deque_node_t *node;
  if (!deque || !deque->first) return NULL;
  node = deque->first;
  item = node->item;
  if (deque->size == 1)  /* If only item */
  {
    deque->first = NULL;
    deque->last = NULL;
  }
  else
  {
    deque->first = node->succeeding;
    deque->first->preceding = NULL;
  }
  deque->size--;
  FreeDequeNode(node);
  return item;
}

void *PopDequeLast(deque_t *deque)
{
  void *item;
  deque_node_t *node;
  if (!deque || !deque->last) return NULL;
  node = deque->last;
  item = node->item;
  if (deque->size == 1)  /* If only item */
  {
    deque->first = NULL;
    deque->last = NULL;
  }
  else
  {
    deque->last = node->preceding;
    deque->last->succeeding = NULL;
  }
  deque->size--;
  FreeDequeNode(node);
  return item;
}

size_t DequeSize(deque_t const *deque)
{
  if (!deque) return 0;
  return deque->size;
}

/* A do-nothing function with the same function type as free(). */
static void nopFree(void * v __unused) { }
void ClearDeque(deque_t *deque)
{
  ClearDequeDestroyItems(deque, nopFree);
}

void ClearDequeFreeItems(deque_t *deque)
{
  ClearDequeDestroyItems(deque, free);
}

void ClearDequeDestroyItems(deque_t *deque, void (*dtor)(void *))
{
  deque_node_t *node, *next;
  if (!deque || !dtor) return;
  /* Destroy all elements of Deque from First to Last */
  node = deque->first;
  while (node)
  {
    /* Only destroy non-null items. */
    if (node->item) dtor(node->item);
    next = node->succeeding;
    FreeDequeNode(node);
    node = next;
  }
  deque->size = 0;
  deque->first = NULL;
  deque->last = NULL;
}

/* - - Deque Node Internal API - - */

static deque_node_t *CreateDequeNode(void *item)
{
  deque_node_t *node;
  node = calloc(1, sizeof(deque_node_t));
  node->item = item;
  return node;
}

static void FreeDequeNode(deque_node_t *node)
{
  memset(node, 0, sizeof(deque_node_t));
  free(node);
}
