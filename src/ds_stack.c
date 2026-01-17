#include "ds_stack.h"
#include "ds_common.h"
#include "ds_deque.h"
#include <stdbool.h>
#include <stdlib.h>

struct ds_stack {
  ds_deque_t *deque; 
};

/**
 * @brief Create a new dynamic stack.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new stack on success, or NULL on failure.
 */
ds_stack_t *ds_stack_create(size_t capacity_hint) {
  ds_stack_t *stack = malloc(sizeof(ds_stack_t));
  if (!stack) return NULL;

  stack->deque = ds_deque_create(capacity_hint);
  if (!stack->deque) {
    free(stack);
    return NULL;
  }

  return stack;
}

/**
 * @brief Destroy a stack and optionally free its elements.
 *
 * @param stack      Pointer to the stack.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the stack itself is freed.
 */
void ds_stack_destroy(ds_stack_t *stack, ds_free_f free_func) {
  if (!stack) return;
  
  ds_deque_destroy(stack->deque, free_func);
  free(stack);
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_stack_size(const ds_stack_t *stack) {
  if (!stack) return 0;

  return ds_deque_size(stack->deque);
}

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_stack_capacity(const ds_stack_t *stack) {
  if (!stack) return 0;

  return ds_deque_capacity(stack->deque);
}

/**
 * @brief Check if the stack is empty.
 */
bool ds_stack_is_empty(ds_stack_t *stack) {
  if (!stack) return true;

  return ds_deque_is_empty(stack->deque);
}

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
ds_status_t ds_stack_push(ds_stack_t *stack, void *element) {
  if (!stack) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  return ds_deque_push_back(stack->deque, element);  
}

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
void *ds_stack_pop(ds_stack_t *stack) {
  if (!stack) return NULL;

  return ds_deque_pop_back(stack->deque);
}

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
void *ds_stack_top(ds_stack_t *stack) {
  if (!stack) return NULL;

  return ds_deque_back(stack->deque);
}

/**
 * @brief Remove all elements from the stack.
 *
 * @param stack      Pointer to the stack.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_stack_clear(ds_stack_t *stack, ds_free_f free_func) {
  if (!stack) return;

  ds_deque_clear(stack->deque, free_func);
}
