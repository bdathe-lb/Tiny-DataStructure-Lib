#ifndef STACK_H
#define STACK_H

#include "ds_common.h"
#include <stddef.h>

/**
 * @brief Opaque stack type.
 *
 * The internal structure is hidden from users.
 * All operations must be performed through the provided API.
 */
typedef struct ds_stack ds_stack_t;

/**
 * @brief Create a new dynamic stack.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new stack on success, or NULL on failure.
 */
ds_stack_t *ds_stack_create(size_t capacity_hint);

/**
 * @brief Destroy a stack and optionally free its elements.
 *
 * @param stack      Pointer to the stack.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the stack itself is freed.
 */
void ds_stack_destroy(ds_stack_t *stack, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_stack_size(const ds_stack_t *stack);

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_stack_capacity(const ds_stack_t *stack);

/**
 * @brief Check if the stack is empty.
 */
bool ds_stack_is_empty(ds_stack_t *stack);

/**
 * @brief Append an element to the end of the stack.
 *
 * @param stack    Pointer to the stack.
 * @param element  Pointer to the element to append.
 *
 * @return
 *   - DS_OK         On success.
 *   - DS_ERR_*      On failure.
 */
ds_status_t ds_stack_push(ds_stack_t *stack, void *element);

/**
 * @brief Remove and return the last element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param stack  Pointer to the stack.
 *
 * @return Pointer to the removed element, or NULL if the stack is empty.
 */
void *ds_stack_pop(ds_stack_t *stack);

/**
 * @brief Access the last element of the queue. 
 *
 * Note that this element is not pop, 
 * and the caller should not free it.
 *
 * @param stack  Pointer to the stack.
 *
 * @return Pointer to the removed element, or NULL if the stack is empty.
 */
void *ds_stack_top(ds_stack_t *stack);

/**
 * @brief Remove all elements from the stack.
 *
 * @param stack      Pointer to the stack.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_stack_clear(ds_stack_t *stack, ds_free_f free_func);

#endif // !STACK_H
