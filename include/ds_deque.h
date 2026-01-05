#ifndef DS_DEQUE_H
#define DS_DEQUE_H

#include "ds_common.h"
#include <stddef.h>

/**
 * @brief Opaque deque type.
 *
 * The internal structure is hidden from users.
 * All operations must be performed through the provided API.
 */
typedef struct ds_deque ds_deque_t;

/**
 * @brief Create a new dynamic deque.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new deque on success, or NULL on failure.
 */
ds_deque_t *ds_deque_create(size_t capacity_hint);

/**
 * @brief Destroy a deque and optionally free its elements.
 *
 * @param deque      Pointer to the deque.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the deque itself is freed.
 */
void ds_deque_destroy(ds_deque_t *deque, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_deque_size(const ds_deque_t *deque);

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_deque_capacity(const ds_deque_t *deque);

/**
 * @brief Check if the deque is empty.
 */
bool ds_deque_is_empty(const ds_deque_t *deque);

/**
 * @brief Append an element to the end of the deque.
 *
 * @param deque    Pointer to the deuqe.
 * @param element  Pointer to the element to append.
 *
 * @return
 *   - DS_OK         On success.
 *   - DS_ERR_BOUNDS Invalid parameters.
 *   - DS_ERR_MEM    Memory allocation fails.
 */
ds_status_t ds_deque_push_back(ds_deque_t *deque, void *element);

/**
 * @brief Append an element to the front of the deque.
 *
 * @param deque    Pointer to the deque.
 * @param element  Pointer to the element to append.
 *
 * @return
 *   - DS_OK         On success.
 *   - DS_ERR_BOUNDS Invalid parameters.
 *   - DS_ERR_MEM    Memory allocation fails.
 */
ds_status_t ds_deque_push_front(ds_deque_t *deque, void *element);

/**
 * @brief Remove and return the last element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param deque  Pointer to the deque.
 *
 * @return Pointer to the removed element, or NULL if the deque is empty.
 */
void *ds_deque_pop_back(ds_deque_t *deque);

/**
 * @brief Remove and return the first element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param deque  Pointer to the deque.
 *
 * @return Pointer to the removed element, or NULL if the deque is empty.
 */
void *ds_deque_pop_front(ds_deque_t *deque);

/**
 * @brief Access the first element of the queue. 
 *
 * Note that this element is not dequeued, 
 * and the caller should not free it.
 *
 * @param deque  Pointer to the deque.
 *
 * @return Pointer to the removed element, or NULL if the deque is empty.
 */
void *ds_deque_front(const ds_deque_t *deque);

/**
 * @brief Access the last element of the queue. 
 *
 * Note that this element is not dequeued, 
 * and the caller should not free it.
 *
 * @param deque  Pointer to the deque.
 *
 * @return Pointer to the removed element, or NULL if the deque is empty.
 */
void *ds_deque_back(const ds_deque_t *deque);

/**
 * @brief Remove all elements from the deque.
 *
 * @param deque      Pointer to the deque.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_deque_clear(ds_deque_t *deque, ds_free_f free_func);

#endif // !DS_DEQUE_H

