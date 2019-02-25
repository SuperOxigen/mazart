
#include <stdlib.h>
#include <string.h>

#include "deque.h"

/* - - Deque Structure - - */

typedef struct deque_node_st deque_node_t;

struct deque_st {
  size_t size;
  deque_node_t *first;
  deque_node_t *last;
};

struct deque_node_st {
  void *item;
  deque_node_t *preceding;  /* Closer to front */
  deque_node_t *succeeding;  /* Closer to back */
};

/* - - Deque Node API Prototypes - - */
static deque_node_t *CreateDequeNode(void *item);
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

bool_t EnqueueFirst(deque_t *deque, void *item)
{
  deque_node_t *node;
  if (!deque) return false;
  node = CreateDequeNode(item);
  if (deque->first)
  {
    deque->first->preceding = node;
    node->succeeding = deque->first;
  }
  else
  {
    deque->last = node;
  }
  deque->first = node;
  deque->size++;
  return true;
}

bool_t EnqueueLast(deque_t *deque, void *item)
{
  deque_node_t *node;
  if (!deque) return false;
  node = CreateDequeNode(item);
  if (deque->last)
  {
    deque->last->succeeding = node;
    node->preceding = deque->last;
  }
  else
  {
    deque->first = node;
  }
  deque->last = node;
  deque->size++;
  return true;
}

void *PeekDequeFirst(deque_t const *deque)
{
  if (!deque || !deque->first) return NULL;
  return deque->first->item;
}

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

void *PeekDequeLast(deque_t const *deque)
{
  if (!deque || !deque->last) return NULL;
  return deque->last->item;
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
  node = deque->first;
  while (node)
  {
    if (node->item) dtor(node->item);
    next = node->succeeding;
    FreeDequeNode(node);
    node = next;
  }
  deque->size = 0;
  deque->first = NULL;
  deque->last = NULL;
}

/* - - Deque Node API - - */

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
