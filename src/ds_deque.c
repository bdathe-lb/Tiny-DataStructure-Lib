#include "ds_deque.h"
#include "ds_common.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>

#define DS_DEQUE_DEFAULT_CAPACITY 16
#define DS_DEQUE_GROWTH_FACTOR     2

/*
 * Next Index: (index + 1) % capacity
 * Prev Index: (index - 1 + capacity) % capacity
 * [head, tail)
 */

struct ds_deque {
  void **items;     // A dynamic array
  size_t capacity;  // Total capacity of array
  size_t size;      // Number of the current array

  size_t head;      // Points to the first valid element
  size_t tail;      // Points to the next writable position 
                    // (one past the tail element)
};

/*
 * @brief Helper inline function for getting the next index in a circular buffer.
 */
static inline size_t next_idx(size_t i, size_t cap) {
    return (i + 1) % cap;
}

/*
 * @brief Helper inline function for obtaining the previous index in a circular buffer.
 */
static inline size_t prev_idx(size_t i, size_t cap) {
    return (i + cap - 1) % cap;
}

/**
 * @brief Expand the capacity of the deque.
 * Allocate a new array and copy the data linearly into it, 
 * then reset head=0 and tail=size
 */
static ds_status_t ds_deque_grow(ds_deque_t *deque) {
  // 1. Allocate a new array
  size_t new_capacity = deque->capacity * DS_DEQUE_GROWTH_FACTOR;
  // Check overflow
  if (new_capacity < deque->capacity) 
    return DS_ERR_MEM;

  void **new_items = malloc(sizeof(void *) * new_capacity);
  if (!new_items) 
    return DS_ERR_MEM;

  // 2. Linearize (Unroll) the buffer
  size_t h = deque->head;
  for (size_t i = 0; i < deque->size; ++ i) {
    new_items[i] = deque->items[h];
    h = next_idx(h, deque->capacity);
  }

  // 3. Reset head and tail
  free(deque->items);
  deque->items = new_items;
  deque->capacity = new_capacity;
  deque->head = 0;
  deque->tail = deque->size;

  return DS_OK;
}

/**
 * @brief Create a new dynamic deque.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new deque on success, or NULL on failure.
 */
ds_deque_t *ds_deque_create(size_t capacity_hint) {
  size_t capacity = capacity_hint == 0 
                    ? DS_DEQUE_DEFAULT_CAPACITY
                    : capacity_hint;

  ds_deque_t *deque = malloc(sizeof(ds_deque_t));
  if (!deque)
    return NULL;

  deque->items = malloc(sizeof(void *) * capacity);
  if (!deque->items) {
    free(deque);
    return NULL;
  }
                    
  deque->capacity = capacity;
  deque->size = 0;

  // [head, tail)
  deque->head = 0;
  deque->tail = 0;

  return deque;
}

/**
 * @brief Destroy a deque and optionally free its elements.
 *
 * @param deque      Pointer to the deque.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the deque itself is freed.
 */
void ds_deque_destroy(ds_deque_t *deque, ds_free_f free_func) {
  // Check input parameters
  if (!deque) return;

  // Traverse each element for free
  if (free_func) {
    size_t h = deque->head;
    for (size_t i = 0; i < deque->size; ++ i) {
      free_func(deque->items[h]);
      h = next_idx(h, deque->capacity);
    }
  }

  free(deque->items);
  free(deque);
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_deque_size(const ds_deque_t *deque) {
  // Check input parameters
  if (!deque) return 0;

  return deque->size;
}

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_deque_capacity(const ds_deque_t *deque) {
  // Check input parameters
  if (!deque) return 0;

  return deque->capacity;
}

/**
 * @brief Check if the deque is empty.
 */
bool ds_deque_is_empty(const ds_deque_t *deque) {
  // Check input parameters
  if (!deque) return true;

  return deque->size == 0;
}

/**
 * @brief Append an element to the end of the deque.
 *
 * @param deque    Pointer to the deuqe.
 * @param element  Pointer to the element to append.
 *
 * @return
 *   - DS_OK         On success.
 *   - DS_ERR_*      On failure.
 */
ds_status_t ds_deque_push_back(ds_deque_t *deque, void *element) {
  // Check input parameters
  if (!deque) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  // Check deque if is full
  if (deque->size == deque->capacity) {
    // Expand capacity
    ds_status_t ret = ds_deque_grow(deque);
    if (ret != DS_OK)
      return ret;
  }

  // Append element
  deque->items[deque->tail] = element;
  deque->tail = next_idx(deque->tail, deque->capacity);
  deque->size ++;
  
  return DS_OK;
}

/**
 * @brief Append an element to the front of the deque.
 *
 * @param deque    Pointer to the deque.
 * @param element  Pointer to the element to append.
 *
 * @return
 *   - DS_OK         On success.
 *   - DS_ERR_*      On failure.
 */
ds_status_t ds_deque_push_front(ds_deque_t *deque, void *element) {
  // Check input parameters
  if (!deque) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  // Check deque if is full
  if (deque->size == deque->capacity) {
    // Expand capacity
    ds_status_t ret = ds_deque_grow(deque);
    if (ret != DS_OK)
      return ret;
  }

  // Append element
  deque->head = prev_idx(deque->head, deque->capacity);
  deque->items[deque->head] = element;
  deque->size ++;
  
  return DS_OK;
}

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
void *ds_deque_pop_back(ds_deque_t *deque) {
  // Check input parameters
  if (!deque) return NULL;

  // Check if the deque is empty
  if (deque->size == 0)
    return NULL;

  deque->tail = prev_idx(deque->tail, deque->capacity);
  deque->size --;
  return deque->items[deque->tail];
}

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
void *ds_deque_pop_front(ds_deque_t *deque) {
  // Check input parameters
  if (!deque) return NULL;

  // Check if the deque is empty
  if (deque->size == 0)
    return NULL;

  void *ret = deque->items[deque->head];
  deque->head = next_idx(deque->head, deque->capacity);
  deque->size --;

  return ret;
}

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
void *ds_deque_front(const ds_deque_t *deque) {
  // Check input parameters
  if (!deque) return NULL;

  // Check if the deque is empty
  if (deque->size == 0) return NULL;

  return deque->items[deque->head];
}

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
void *ds_deque_back(const ds_deque_t *deque) {
  // Check input parameters
  if (!deque) return NULL;

  // Check if the deque is empty
  if (deque->size == 0) return NULL;

  return deque->items[prev_idx(deque->tail, deque->capacity)];
}

/**
 * @brief Remove all elements from the deque.
 *
 * @param deque      Pointer to the deque.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_deque_clear(ds_deque_t *deque, ds_free_f free_func) {
  // Check input parameters
  if (!deque) return;

  // Traverse each element for free
  if (free_func) {
    size_t h = deque->head;
    for (size_t i = 0; i < deque->size; ++ i) {
      free_func(deque->items[h]);
      h = next_idx(h, deque->capacity);
    }
  }

  deque->size = 0;
  deque->head = 0;
  deque->tail = 0;
}
