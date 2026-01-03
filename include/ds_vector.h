/*
** src/ds_vector.h -- A simple dynamic array implementation storing opaque pointers (void *).
**                    Provides basic vector operations with explicit ownership control.
*/

#ifndef DS_VECTOR_H
#define DS_VECTOR_H

#include "ds_common.h"

/**
 * @brief Opaque vector type.
 *
 * The internal structure is hidden from users.
 * All operations must be performed through the provided API.
 */
typedef struct ds_vector ds_vector_t;

/**
 * @brief Create a new dynamic vector.
 *
 * @param capacity_hint  Suggested initial capacity. If zero, a default
 *                       capacity is used.
 *
 * @return Pointer to a new vector on success, or NULL on failure.
 */
ds_vector_t *ds_vector_create(size_t capacity_hint);

/**
 * @brief Destroy a vector and optionally free its elements.
 *
 * @param vec        Pointer to the vector.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the vector itself is freed.
 */
void ds_vector_destroy(ds_vector_t *vec, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_vector_size(const ds_vector_t *vec);

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_vector_capacity(const ds_vector_t *vec);

/**
 * @brief Check if the vector is empty.
 */
bool ds_vector_is_empty(const ds_vector_t *vec);

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
ds_status_t ds_vector_reserve(ds_vector_t *vec, size_t new_capacity);

/**
 * @brief Get the element at the specified index.
 *
 * @param vec    Pointer to the vector.
 * @param index  Element index.
 *
 * @return Pointer to the element, or NULL if index is out of range.
 */
void *ds_vector_get(const ds_vector_t *vec, size_t index);

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
ds_status_t ds_vector_set(ds_vector_t *vec, size_t index, void *element, ds_free_f old_element_free);

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
ds_status_t ds_vector_push_back(ds_vector_t *vec, void *element);

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
ds_status_t ds_vector_insert(ds_vector_t *vec, size_t index, void *element);

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
void *ds_vector_pop_back(ds_vector_t *vec);

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
ds_status_t ds_vector_remove(ds_vector_t *vec, size_t index, ds_free_f free_func);

/**
 * @brief Remove all elements from the vector.
 *
 * @param vec        Pointer to the vector.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_vector_clear(ds_vector_t *vec, ds_free_f free_func);

#endif // !DS_VECTOR_H
