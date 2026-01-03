/*
** src/ds_vector.c -- Implementation of the ds_vector dynamic array.
**                    Handles memory management, resizing, and element operations.
*/

#include "ds_vector.h"
#include "ds_common.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DS_VECTOR_DEFAULT_CAPACITY 16
#define DS_VECTOR_GROWTH_FACTOR     2

typedef struct ds_vector {
  void **items;     // Dynamic array, store `void *` pointer
  size_t capacity;  // Capacity of current array
  size_t size;      // Element count of current array
} ds_vector_t;

/**
 * @brief Create a new dynamic vector.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new vector on success, or NULL on failure.
 */
ds_vector_t *ds_vector_create(size_t capacity_hint) {
  // Check input paraments
  size_t capacity = \
    capacity_hint == 0 \
    ? DS_VECTOR_DEFAULT_CAPACITY \
    : capacity_hint;
  
  // Allocate memory spaces for `ds_vector_t`
  ds_vector_t *vec = malloc(sizeof(ds_vector_t));
  if (!vec) 
    return NULL;

  // Allocate memory spaces for `items`
  vec->items = malloc(sizeof(void *) * capacity);
  if (!vec->items) {
    free(vec);
    return NULL;
  }

  vec->size = 0;
  vec->capacity = capacity;
  
  // Success
  return vec;
}

/**
 * @brief Destroy a vector and optionally free its elements.
 *
 * @param vec        Pointer to the vector.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the vector itself is freed.
 */
void ds_vector_destroy(ds_vector_t *vec, ds_free_f free_func) {
  // Check input paraments
  if (!vec)
    return;

  // A specific function is passed
  if (free_func) {
    for (size_t i = 0; i < vec->size; ++ i) {
      free_func(vec->items[i]);
    }
  }

  // Free the memory of the vector
  free(vec->items);
  free(vec);
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_vector_size(const ds_vector_t *vec) {
  // Check input paraments
  if (!vec)
    return 0;

  return vec->size;
}

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_vector_capacity(const ds_vector_t *vec) {
  // Check input paraments
  if (!vec)
    return 0;
  
  return vec->capacity;
}

/**
 * @brief Check if the vector is empty.
 */
bool ds_vector_is_empty(const ds_vector_t *vec) {
  // Check input paraments
  if (!vec)
    return true;
  
  return vec->size == 0;
}

/**
 * @brief Reserve capacity for at least new_capacity elements.
 *
 * This function may reallocate internal storage to reduce future
 * reallocations.
 *
 * @param vec           Pointer to the vector.
 * @param new_capacity  Desired minimum capacity.
 *
 * @return
 *   - DS_OK on success, or if new_capacity is less than or equal to
 *     the current capacity.
 *   - DS_ERR_BOUNDS if new_capacity is smaller than the current size.
 *   - DS_ERR_MEM if memory allocation fails.
 */
ds_status_t ds_vector_reserve(ds_vector_t *vec, size_t new_capacity) {
  // Check input paraments
  if (!vec || new_capacity < vec->size) 
    return DS_ERR_BOUNDS;

  if (new_capacity <= vec->capacity)
    return DS_OK;

  void **new_items = realloc(vec->items, sizeof(void *) * new_capacity);
  if (!new_items)
    return DS_ERR_MEM;
  
  vec->items = new_items;
  vec->capacity = new_capacity;
  
  return DS_OK;
}

/**
 * @brief Get the element at the specified index.
 *
 * @param vec    Pointer to the vector.
 * @param index  Element index.
 *
 * @return Pointer to the element, or NULL if index is out of range.
 */
void *ds_vector_get(const ds_vector_t *vec, size_t index) {
  // Check input paraments 
  if (!vec || index >= vec->size)
    return NULL;

  return vec->items[index];
}

/**
 * @brief Replace the element at the specified index. 
 *
 * @param vec               Pointer to the vector.
 * @param index             Element index.
 * @param element           Pointer to the new element.
 * @param old_element_free  Optional destructor for the old element.
 *                          - If non-NULL, it is called before replacement.
 *                          - If NULL, the old element pointer is overwritten
 *                            without freeing.
 *
 * @return
 *   - DS_OK on success.
 *   - DS_ERR_BOUNDS if index is out of range.
 */
ds_status_t ds_vector_set(ds_vector_t *vec, size_t index, void *element, ds_free_f old_element_free) {
  // Check input paraments
  if (!vec || index >= vec->size || !element)
    return DS_ERR_BOUNDS;
  
  // Replace first, destroy later
  // Copy -> Update -> Destroy
  void *old_element = vec->items[index];

  vec->items[index] = element;

  if (old_element_free) {
    old_element_free(old_element);
  }

  return DS_OK;
}

/**
 * @brief Append an element to the end of the vector.
 *
 * @param vec      Pointer to the vector.
 * @param element  Pointer to the element to append.
 *
 * @return
 *   - DS_OK on success.
 *   - DS_ERR_MEM if memory allocation fails.
 */
ds_status_t ds_vector_push_back(ds_vector_t *vec, void *element) {
  // Check input paraments
  if (!vec || !element)
    return DS_ERR_BOUNDS;

  return ds_vector_insert(vec, vec->size, element);
}

/**
 * @brief Insert an element at the specified position.
 *
 * @param vec      Pointer to the vector.
 * @param index    Insertion position in the range [0, size].
 *                 An index equal to the current size is equivalent
 *                 to ds_vector_push_back().
 * @param element  Pointer to the element to insert.
 *
 * @return
 *   - DS_OK on success.
 *   - DS_ERR_BOUNDS if index is out of range.
 *   - DS_ERR_MEM if memory allocation fails.
 */
ds_status_t ds_vector_insert(ds_vector_t *vec, size_t index, void *element) {
  // Check input paraments
  if (!vec ||index > vec->size || !element)
    return DS_ERR_BOUNDS;

  // Check the capacity of vector
  if (vec->size == vec->capacity) {
    // Capacity expansion
    int growth_factor = DS_VECTOR_GROWTH_FACTOR;
    size_t new_capacity = vec->capacity * growth_factor;

    ds_status_t ret = ds_vector_reserve(vec, new_capacity);
    if (ret != DS_OK)
      return ret;
  }

  // Move elements
  if (index < vec->size) {
    memmove(&vec->items[index+1], 
            &vec->items[index],
            (vec->size - index) * sizeof(void *));
  }

  vec->items[index] = element;
  vec->size ++;

  return DS_OK;
}

/**
 * @brief Remove and return the last element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param vec  Pointer to the vector.
 *
 * @return Pointer to the removed element, or NULL if the vector is empty.
 */
void *ds_vector_pop_back(ds_vector_t *vec) {
  // Check input paraments
  if (!vec || vec->size == 0) 
    return NULL;

  void *val = vec->items[vec->size-1];
  vec->size --;

  return val;
}

/**
 * @brief Remove the element at the specified index.
 *
 * @param vec        Pointer to the vector.
 * @param index      Element index.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called on the removed element.
 *                   - If NULL, the element pointer is removed without
 *                   freeing the associated memory.
 *
 * @return
 *   - DS_OK on success.
 *   - DS_ERR_BOUNDS if index is out of range.
 */

ds_status_t ds_vector_remove(ds_vector_t *vec, size_t index, ds_free_f free_func) {
  // Check input paraments 
  if (!vec || index >= vec->size) 
    return DS_ERR_BOUNDS;

  // Copy
  void *tmp = vec->items[index];
  
  // Move
  memmove(&vec->items[index], 
          &vec->items[index+1],
          (vec->size - index - 1) * sizeof(void *));

  // Free
  if (free_func) {
    free_func(tmp);
  }

  vec->size --;

  return DS_OK;
}

/**
 * @brief Remove all elements from the vector.
 *
 * @param vec        Pointer to the vector.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_vector_clear(ds_vector_t *vec, ds_free_f free_func) {
  // Check input paraments
  if (!vec)
    return;

  if (free_func) {
    for (size_t i = 0; i < vec->size; ++ i) {
      free_func(vec->items[i]);
    }
  }

  vec->size = 0;

  return;
}
