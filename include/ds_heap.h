#ifndef DS_HEAP_H
#define DS_HEAP_H

#include "ds_common.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Opaque heap type.
 *
 * The internal structure is hidden from users.
 * All operations must be performed through the provided API.
 */
typedef struct ds_heap ds_heap_t;


/**
 * @brief Create a binary heap, 
 *        whose type is determined by the passed comparision function.
 *
 * @param compare        Used to determine if the heap is a max-heap or min-heap.
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new heap on success, or NULL on failure.
 */
ds_heap_t *ds_heap_create(ds_compare_f compare, size_t capacity_hint);

/**
 * @brief Destroy a heap and optionally free its elements.
 *
 * @param heap       Pointer to the heap.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the vector itself is freed.
 */
void ds_heap_destroy(ds_heap_t *heap, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_heap_size(const ds_heap_t *heap);

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_heap_capacity(const ds_heap_t *heap);

/**
 * @brief Check if the heap is empty.
 */
bool ds_heap_is_empty(const ds_heap_t *heap);

/**
 * @brief Insert a new element into the heap.
 *
 * @param heap     Pointer to the heap.
 * @param element  Pointer to the element to insert.
 *
 * @return
 *   - DS_OK         On success.
 *   - DS_ERR_*      On failure.
 */
ds_status_t ds_heap_push(ds_heap_t *heap, void *element);

/**
 * @brief Remove the top element of the heap and return that element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param heap  Pointer to the heap.
 *
 * @return Pointer to the removed element, or NULL if the heap is empty.
 */
void *ds_heap_pop(ds_heap_t *heap);

/**
 * @brief Get the top element of the heap.
 *
 * The user should not free this memory.
 *
 * @param heap  Pointer to the heap.
 *
 * @return Pointer to the removed element, or NULL if the heap is empty.
 */
void *ds_heap_top(ds_heap_t *heap);

/**
 * @brief Remove all elements from the heap.
 *
 * @param heap       Pointer to the heap.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_heap_clear(ds_heap_t *heap, ds_free_f free_func);

#endif // !DS_HEAP_H
