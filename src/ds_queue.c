#include "ds_queue.h"
#include "ds_common.h"
#include "ds_deque.h"
#include <stdbool.h>
#include <stdlib.h>

struct ds_queue {
  ds_deque_t *deque;
};

/**
 * @brief Create a new dynamic queue.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new queue on success, or NULL on failure.
 */
ds_queue_t *ds_queue_create(size_t capacity_hint) {
  ds_queue_t *queue = malloc(sizeof(ds_queue_t));
  if (!queue) return NULL;

  queue->deque = ds_deque_create(capacity_hint);
  if (!queue->deque) {
    free(queue);
    return NULL;
  } 

  return queue;
}

/**
 * @brief Destroy a queue and optionally free its elements.
 *
 * @param queue      Pointer to the queue.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the queue itself is freed.
 */
void ds_queue_destroy(ds_queue_t *queue, ds_free_f free_func) {
  if (!queue) return;

  ds_deque_destroy(queue->deque, free_func);
  free(queue);
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_queue_size(const ds_queue_t *queue) {
  if (!queue) return 0;

  return ds_deque_size(queue->deque);
}

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_queue_capacity(const ds_queue_t *queue) {
  if (!queue) return 0;

  return ds_deque_capacity(queue->deque);
}

/**
 * @brief Check if the queue is empty.
 */
bool ds_queue_is_empty(ds_queue_t *queue) {
  if (!queue) return true;

  return ds_deque_is_empty(queue->deque);
}

/**
 * @brief Append an element to the end of the queue.
 *
 * @param queue    Pointer to the queue.
 * @param element  Pointer to the element to append.
 *
 * @return
 *   - DS_OK         On success.
 *   - DS_ERR_*      On failure.
 */
ds_status_t ds_queue_push(ds_queue_t *queue, void *element) {
  if (!queue) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  return ds_deque_push_back(queue->deque, element);
}

/**
 * @brief Remove and return the first element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param queue  Pointer to the queue.
 *
 * @return Pointer to the removed element, or NULL if the queue is empty.
 */
void *ds_queue_pop(ds_queue_t *queue) {
  if (!queue) return NULL;

  return ds_deque_pop_front(queue->deque);
}

/**
 * @brief Access the first element of the queue. 
 *
 * Note that this element is not pop, 
 * and the caller should not free it.
 *
 * @param queue  Pointer to the queue.
 *
 * @return Pointer to the removed element, or NULL if the queue is empty.
 */
void *ds_queue_front(ds_queue_t *queue) {
  if (!queue) return NULL;

  return ds_deque_front(queue->deque);
}

/**
 * @brief Remove all elements from the queue.
 *
 * @param queue      Pointer to the queue.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_queue_clear(ds_queue_t *queue, ds_free_f free_func) {
  if (!queue) return;

  ds_deque_clear(queue->deque, free_func);
}
