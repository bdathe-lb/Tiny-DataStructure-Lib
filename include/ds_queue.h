#ifndef QUEUE_H
#define QUEUE_H

#include "ds_common.h"
#include <stddef.h>

/**
 * @brief Opaque queue type.
 *
 * The internal structure is hidden from users.
 * All operations must be performed through the provided API.
 */
typedef struct ds_queue ds_queue_t;

/**
 * @brief Create a new dynamic queue.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new queue on success, or NULL on failure.
 */
ds_queue_t *ds_queue_create(size_t capacity_hint);

/**
 * @brief Destroy a queue and optionally free its elements.
 *
 * @param queue      Pointer to the queue.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the queue itself is freed.
 */
void ds_queue_destroy(ds_queue_t *queue, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_queue_size(const ds_queue_t *queue);

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_queue_capacity(const ds_queue_t *queue);

/**
 * @brief Check if the queue is empty.
 */
bool ds_queue_is_empty(ds_queue_t *queue);

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
ds_status_t ds_queue_push(ds_queue_t *queue, void *element);

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
void *ds_queue_pop(ds_queue_t *queue);

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
void *ds_queue_front(ds_queue_t *queue);

/**
 * @brief Remove all elements from the queue.
 *
 * @param queue      Pointer to the queue.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_queue_clear(ds_queue_t *queue, ds_free_f free_func);

#endif // !QUEUE_H
