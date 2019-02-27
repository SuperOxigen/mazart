/*
 * Mazart - Priority Queue
 *  Module provides a Priority Queue data structure.
 *
 * Copyright (c) 2019 Alex Dale
 * This project is licensed under the terms of the MIT license.
 * See LICENSE for details.
 */
#include "priority.h"

#include <stdlib.h>
#include <string.h>

static size_t const kDefaultCapacity = 1024;

/* - - Priority Queue Structure - - */

typedef struct priority_node_st priority_node_t;

struct priority_queue_st {
  priority_node_t *heap;
  size_t size;
  size_t capacity;
};

struct priority_node_st {
  void *item;
  size_t priority;
};

/* - - Priority Queue Node API Prototypes - - */
static priority_node_t *CreatePriorityQueueHeap(size_t capacity);
static priority_node_t *RecreatePriorityQueueHeap(
  priority_node_t *old_heap, size_t old_capacity, size_t new_capacity);
static void FreePriorityQueueHeap(priority_node_t *heap, size_t capacity);

/* - - Priority Queue Internal API Prototypes - - */
static priority_node_t *GetNode(priority_queue_t const *queue, size_t qidx);
static void SetNode(priority_queue_t *queue, size_t qidx, size_t priority, void *item);
static void SwapNodes(priority_queue_t *queue, size_t a, size_t b);

static size_t GetInsertIndex(priority_queue_t const *queue);
static size_t GetLastIndex(priority_queue_t const *queue);
static size_t GetRightChildIndex(priority_queue_t const *queue, size_t qidx);
static size_t GetLeftChildIndex(priority_queue_t const *queue, size_t qidx);
static size_t GetParentIndex(priority_queue_t const *queue, size_t qidx);

static void DownHeap(priority_queue_t *queue, size_t qidx);
static void UpHeap(priority_queue_t *queue, size_t qidx);

/* - - Priority Queue API - - */

priority_queue_t *CreatePriorityQueue(void)
{
  priority_queue_t *queue;
  queue = calloc(1, sizeof(priority_queue_t));
  queue->heap = CreatePriorityQueueHeap(kDefaultCapacity);
  queue->capacity = kDefaultCapacity;
  return queue;
}

void FreePriorityQueue(priority_queue_t *queue)
{
  if (!queue) return;
  FreePriorityQueueHeap(queue->heap, queue->capacity);
  memset(queue, 0, sizeof(priority_queue_t));
  free(queue);
}

bool_t EnqueuePriority(priority_queue_t *queue, size_t priority, void *item)
{
  size_t qidx;
  if (!queue) return false;
  if (queue->size == queue->capacity)
  {
    /* Double heap capacity */
    queue->heap = RecreatePriorityQueueHeap(
      queue->heap, queue->capacity, queue->capacity * 2);
    queue->capacity *= 2;
  }
  qidx = GetInsertIndex(queue);
  SetNode(queue, qidx, priority, item);
  queue->size++;
  UpHeap(queue, qidx);
  return true;
}

void *PeekTopPriority(priority_queue_t const *queue)
{
  if (!queue || queue->size == 0) return NULL;
  return GetNode(queue, 1)->item;
}

void *PopTopPriority(priority_queue_t *queue)
{
  size_t qidx;
  void *item;
  if (!queue || queue->size == 0) return NULL;
  item = GetNode(queue, 1)->item;
  /* Remove Node */
  qidx = GetLastIndex(queue);
  SwapNodes(queue, 1, qidx);
  queue->size--;
  DownHeap(queue, 1);
  return item;
}

size_t PriorityQueueSize(priority_queue_t const *queue)
{
  if (!queue) return 0;
  return queue->size;
}

static void nopFree(void * v __unused) { }

void ClearPriorityQueue(priority_queue_t *queue)
{
  ClearPriorityQueueDestroyItems(queue, nopFree);
}

void ClearPriorityQueueFreeItems(priority_queue_t *queue)
{
  ClearPriorityQueueDestroyItems(queue, free);
}

void ClearPriorityQueueDestroyItems(priority_queue_t *queue, void (*dtor)(void *))
{
  size_t idx;
  if (!queue || !dtor) return;
  for (idx = 0; idx < queue->size; idx++)
  {
    if (queue->heap[idx].item) dtor(queue->heap[idx].item);
  }
  memset(queue->heap, 0, sizeof(priority_node_t) * queue->capacity);
  queue->size = 0;
}

/* - - Priority Queue Internal API - - */

static priority_node_t *GetNode(priority_queue_t const *queue, size_t qidx)
{
  if (qidx == 0) return NULL;
  return &queue->heap[qidx - 1];
}

static void SetNode(priority_queue_t *queue, size_t qidx, size_t priority, void *item)
{
  if (qidx == 0) return;
  queue->heap[qidx - 1].item = item;
  queue->heap[qidx - 1].priority = priority;
}

static void SwapNodes(priority_queue_t *queue, size_t a, size_t b)
{
  size_t temp_priority;
  void *temp_item;
  if (a == 0 || b == 0 || a == b) return;
  temp_priority = queue->heap[a - 1].priority;
  temp_item = queue->heap[a - 1].item;
  queue->heap[a - 1].priority = queue->heap[b - 1].priority;
  queue->heap[a - 1].item = queue->heap[b - 1].item;
  queue->heap[b - 1].priority = temp_priority;
  queue->heap[b - 1].item = temp_item;
}

static size_t GetInsertIndex(priority_queue_t const *queue)
{
  return queue->size + 1;
}

static size_t GetLastIndex(priority_queue_t const *queue)
{
  return queue->size;
}

static size_t GetLeftChildIndex(priority_queue_t const *queue, size_t qidx)
{
  size_t idx;
  if (qidx == 0) return 0;
  idx = qidx * 2;
  if (idx > queue->size) return 0; /* Out of bounds */
  return idx;
}

static size_t GetRightChildIndex(priority_queue_t const *queue, size_t qidx)
{
  size_t idx;
  if (qidx == 0) return 0;
  idx = qidx * 2 + 1;
  if (idx > queue->size) return 0; /* Out of bounds */
  return idx;
}

static size_t GetParentIndex(priority_queue_t const *queue __unused, size_t qidx)
{
  return qidx / 2;
}

static void DownHeap(priority_queue_t *queue, size_t qidx)
{
  size_t lidx, ridx;
  while (true)
  {
    lidx = GetLeftChildIndex(queue, qidx);
    ridx = GetRightChildIndex(queue, qidx);
    /* Check if done */
    if (lidx == 0 && ridx == 0) return;
    /* If ridx is 0, then there is no right child, but there is a
     * left child.  This left child cannot have any children.
     */
    if (ridx == 0)
    {
      if (GetNode(queue, qidx)->priority < GetNode(queue, lidx)->priority)
      {
        SwapNodes(queue, qidx, lidx);
      }
      return;
    }
    if (GetNode(queue, qidx)->priority < GetNode(queue, ridx)->priority)
    {
      SwapNodes(queue, qidx, ridx);
      qidx = ridx;
      continue;
    }
    if (GetNode(queue, qidx)->priority < GetNode(queue, lidx)->priority)
    {
      SwapNodes(queue, qidx, lidx);
      qidx = lidx;
      continue;
    }
    return;
  }
}

static void UpHeap(priority_queue_t *queue, size_t qidx)
{
  size_t pidx;
  while (true)
  {
    pidx = GetParentIndex(queue, qidx);
    if (pidx == 0) return;
    if (GetNode(queue, qidx)->priority > GetNode(queue, pidx)->priority)
    {
      SwapNodes(queue, qidx, pidx);
      qidx = pidx;
      continue;
    }
    return;
  }
}

/* - - Priority Queue Node API - - */
static priority_node_t *CreatePriorityQueueHeap(size_t capacity)
{
  priority_node_t *heap;
  if (capacity == 0) return NULL;
  heap = calloc(capacity, sizeof(priority_node_t));
  return heap;
}

static priority_node_t *RecreatePriorityQueueHeap(
  priority_node_t *old_heap, size_t old_capacity, size_t new_capacity)
{
  priority_node_t *new_heap;
  new_heap = CreatePriorityQueueHeap(new_capacity);
  if (new_heap && old_heap && old_capacity > 0 && new_capacity > 0)
  {
    memcpy(new_heap, old_heap,
      sizeof(priority_node_t) * ((new_capacity > old_capacity) ? old_capacity : new_capacity));
  }
  FreePriorityQueueHeap(old_heap, old_capacity);
  return new_heap;
}

static void FreePriorityQueueHeap(priority_node_t *heap, size_t capacity)
{
  if (!heap) return;
  memset(heap, 0, capacity * sizeof(priority_node_t));
  free(heap);
}
