#ifndef DS_LIST_H
#define DS_LIST_H

#include "ds_common.h"

/**
 * @brief Opaque list node type.
 * 
 * Node is doubly linked list.
 */
typedef struct ds_list_node ds_list_node_t;

/**
 * @brief Opaque list type.
 *
 * The internal structure is hidden from users.
 * All operations must be performed through the provided API.
 * Any non-NULL value represents an iterator pointing to valid data.
 */
typedef struct ds_list ds_list_t;

/**
 * @brief List Iterator.
 *
 * Define it as a pointer so that it can be passed by value.
 * NULL represent End Iterator
 */
typedef ds_list_node_t* ds_list_iter_t;

/**
 * @brief Create a new doubly linked list.
 *
 * @return Pointer to a new list on success, or NULL on failure.
 */
ds_list_t *ds_list_create();

/**
 * @brief Destroy a list and optionally free its elements.
 *
 * @param list       Pointer to the list.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the list itself is freed.
 */
void ds_list_destroy(ds_list_t *list, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_list_size(const ds_list_t *list);

/**
 * @brief Check if the list is empty.
 */
bool ds_list_is_empty(const ds_list_t *list);

/**
 * @brief Replace the element at the specified iterator.
 *
 * @param list              Pointer to the list.
 * @param it                Element node iterator.
 * @param element           Pointer to the new element.
 * @param old_element_free  Optional destructor for the old element.
 *                          - If non-NULL, it is called before replacement.
 *                          - If NULL, the old element pointer is overwritten
 *                            without freeing.
 *
 * @return 
 *  - DS_OK         On success.
 *  - DS_ERR_*      On failure.
 */
ds_status_t ds_list_set(ds_list_t *list, ds_list_iter_t it, void *element, ds_free_f old_element_free);

/**
 * @brief Append an element to the end of the list.
 *
 * @param list     Pointer to the list.
 * @param element  Pointer to the element to append.
 *
 * @return 
 *  - DS_OK         On success.
 *  - DS_ERR_*      On failure.
 */
ds_status_t ds_list_push_back(ds_list_t *list, void *element);

/**
 * @brief Append an element to the front of the list.
 *
 * @param list     Pointer to the list.
 * @param element  Pointer to the element to append.
 *
 * @return 
 *  - DS_OK         On success.
 *  - DS_ERR_*      On failure.
 */
ds_status_t ds_list_push_front(ds_list_t *list, void *element);

/**
 * @brief Insert an element at the specified position.
 *
 * @param list     Pointer to the list.
 * @param it       Iterator to the target position.
 *                 - If it == ds_list_iter_begin(list), equivalent to push_front.
 *                 - If it == ds_list_iter_end(list) (i.e., NULL), equivalent to push_back.
 *                 - Otherwise, insert a new node before the node pointed to by it.
 * @param element  Pointer to the element to insert.
 *
 * @return 
 *  - DS_OK         On success.
 *  - DS_ERR_*      On failure.
 */
ds_status_t ds_list_insert(ds_list_t *list, ds_list_iter_t it, void *element);

/**
 * @brief Remove and return the last element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param list  Pointer to the list.
 *
 * @return Pointer to the removed element, or NULL if the list is empty.
 */
void *ds_list_pop_back(ds_list_t *list);

/**
 * @brief Remove and return the first element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param list  Pointer to the list.
 *
 * @return Pointer to the removed element, or NULL if the list is empty.
 */
void *ds_list_pop_front(ds_list_t *list);

/**
 * @brief Remove the element at the specified index.
 *
 * @param list       Pointer to the list.
 * @param it         Element node iterator.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called on the removed element.
 *                   - If NULL, the element pointer is removed without
 *                     freeing the associated memory.
 *
 * @return ds_list_iter_t The iterator pointing to the element after the removed one.
 *                        Returns NULL if the last element was removed.
 */
ds_list_iter_t ds_list_remove(ds_list_t *list, ds_list_iter_t it, ds_free_f free_func);

/**
 * @brief Remove all elements from the list.
 *
 * @param list       Pointer to the list.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_list_clear(ds_list_t *list, ds_free_f free_func);

/**
 * @brief Get an iterator pointing to the first element
 *
 * @param list Pointer to the list.
 *
 * @return Returns NULL (End) if the list is empty.
 */
ds_list_iter_t ds_list_iter_begin(ds_list_t *list);

/**
 * @brief Get an iterator pointing to "End" (Sentinel)
 *
 * @param list Pointer to the list.
 *
 * @return Always returns NULL. Used to determine whether traversal has ended.
 */
ds_list_iter_t ds_list_iter_end(ds_list_t *list);

/**
 * @brief Get an iterator pointing to the last valid element (Tail)
 *
 * @note This is a helper function to support reverse traversal.
 *
 * @return Returns NULL if the list is empty.
 */
ds_list_iter_t ds_list_iter_tail(ds_list_t *list);

/**
 * @brief Get the next element at the current iterator position and 
 *        move forward by one position.
 *
 * @param Pointer to an iterator of type ds_list_iter_t.
 *
 * @return Pointer to the iterator.
 */
ds_list_iter_t ds_list_iter_next(ds_list_iter_t it);

/**
 * @brief Get the previous element at the current iterator position and 
 *        move backward by one position.
 *
 * @param Pointer to an iterator of type ds_list_iter_t.
 *
 * @return Pointer to the iterator.
 */
ds_list_iter_t ds_list_iter_prev(ds_list_iter_t it);

/**
 * @brief Get the data pointed to by the iterator.
 *
 * @param Pointer to an iterator of type ds_list_iter_t.
 *
 * @return Data pointed to by the iterator.
 */
void *ds_list_iter_get(ds_list_iter_t it);

/**
 * @brief Determine if two iterators are equal.
 *
 * @param Two iterators to be compared.
 *
 * @return Returns true if they are equal, otherwise false.
 */
bool ds_list_iter_equal(const ds_list_iter_t a, const ds_list_iter_t b);

#endif // !DS_LIST_H
