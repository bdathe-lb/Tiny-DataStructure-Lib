#include "ds_list.h"
#include "ds_common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

// NULL <- [head] <-> [node] <-> [node] <-> [tail] -> NULL
struct ds_list_node {
  void *data;
  struct ds_list_node *prev;
  struct ds_list_node *next;
};

struct ds_list {
  ds_list_node_t *head;
  ds_list_node_t *tail;
  size_t size;
};

/**
 * @brief Create a new doubly linked list.
 *
 * @return Pointer to a new list on success, or NULL on failure.
 */
ds_list_t *ds_list_create() {
  ds_list_t *list = malloc(sizeof(ds_list_t));
  if (!list) return NULL;

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;

  return list;
}

/**
 * @brief Destroy a list and optionally free its elements.
 *
 * @param list       Pointer to the list.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the list itself is freed.
 */
void ds_list_destroy(ds_list_t *list, ds_free_f free_func) {
  // Check input parameters
  if (!list) return;
  
  // Iterate each node for free
  ds_list_node_t *p = list->head;
  while (p != NULL) {
    if (free_func)
      free_func(p->data);

    // Copy
    ds_list_node_t *tmp = p;
    // Update
    p = p->next;
    // Free
    free(tmp);
  }

  free(list);
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_list_size(const ds_list_t *list) {
  // Check input parameters
  if (!list) return 0;

  return list->size;
}

/**
 * @brief Check if the list is empty.
 */
bool ds_list_is_empty(const ds_list_t *list) {
  // Check input parameters
  if (!list) return true;

  return list->size == 0;
}

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
ds_status_t ds_list_set(ds_list_t *list, ds_list_iter_t it, void *element, ds_free_f old_element_free) {
  // Check input parameters
  if (!list) return DS_ERR_NULL;
  if (!it || !element) return DS_ERR_ARG;
  
  // Copy
  void *old_element = it->data;
  // Update
  it->data = element;
  // Free
  if (old_element_free) old_element_free(old_element);

  return DS_OK;
}

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
ds_status_t ds_list_push_back(ds_list_t *list, void *element) { 
  // Check input parameters
  if (!list) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  return ds_list_insert(list, NULL, element);
}

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
ds_status_t ds_list_push_front(ds_list_t *list, void *element) {
  // Check input parameters
  if (!list) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  return ds_list_insert(list, list->head, element);
}

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
ds_status_t ds_list_insert(ds_list_t *list, ds_list_iter_t it, void *element) {
  // Check input parameters
  if (!list) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  // Construct a new node
  ds_list_node_t *node = malloc(sizeof(ds_list_node_t));
  if (!node)
    return DS_ERR_MEM;

  // Case 1: list is empty
  if (list->size == 0) {
    node->data = element;
    node->prev = NULL;
    node->next = NULL; 
    list->head = node;
    list->tail = node;
  }
  // Case 2: it == head
  else if (it == list->head) {
    node->data = element;
    node->next = list->head;
    node->prev = NULL;
    list->head->prev = node;

    list->head = node;
  } 
  // Case 3: it == end(NULL)
  else if (it == NULL) {
    node->data = element;
    node->next = NULL;
    node->prev = list->tail;
    list->tail->next = node;

    list->tail = node;
  }
  // Case 4: Otherwise
  else {
    ds_list_node_t *p = it;
    ds_list_node_t *q = it->prev;
    node->data = element;
    node->next = p;
    node->prev = q;
    q->next = node;
    p->prev = node;
  }

  list->size ++;

  return DS_OK;
}

/**
 * @brief Remove and return the last element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param list Pointer to the list.
 *
 * @return Pointer to the removed element, or NULL if the list is empty.
 */
void *ds_list_pop_back(ds_list_t *list) {
  // Check input parameters
  if (!list || list->size == 0) return NULL;

  // Copy
  ds_list_node_t *tmp = list->tail;
  void *ret = tmp->data;
  
  // Case 1: element count of list equal to 1
  if (list->size == 1) {
    // Update
    list->head = NULL;
    list->tail = NULL;
  }
  // Case 2: Otherwise
  else {
    // Update
    tmp->prev->next = NULL;
    list->tail = tmp->prev;
  }

  // Free
  free(tmp);

  list->size --;
  return ret;
}

/**
 * @brief Remove and return the first element.
 *
 * Ownership of the returned pointer is transferred to the caller.
 * The element is not automatically freed.
 *
 * @param list       Pointer to the list.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called on the removed element.
 *                   - If NULL, the element pointer is removed without
 *                     freeing the associated memory.
 *
 * @return Pointer to the removed element, or NULL if the list is empty.
 */
void *ds_list_pop_front(ds_list_t *list) {
  // Check input parameters
  if (!list || list->size == 0) return NULL;

  // Copy
  ds_list_node_t *tmp = list->head;
  void *ret = tmp->data;

  // Case 1: element count of list equal to 1
  if (list->size == 1) {
    // Update
    list->head = NULL;
    list->tail = NULL;
  } 
  // Case 2: Otherwise
  else {
    // Update
    tmp->next->prev = NULL;
    list->head = tmp->next;
  }

  // Free
  free(tmp);
  list->size --;

  return ret;
}

/**
 * @brief Remove the element at the specified iterator.
 *
 * @param list       Pointer to the list.
 * @param it         Iterator to the target position.
 *                   - If it points to a valid element, that element is removed.
 *                   - If it == ds_list_iter_end(list) (i.e., NULL), no operation is performed.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called on the removed element.
 *                   - If NULL, the element pointer is removed without
 *                     freeing the associated memory.
 *
 * @return ds_list_iter_t The iterator pointing to the element after the removed one.
 *                        Returns NULL if the last element was removed.
 */
ds_list_iter_t ds_list_remove(ds_list_t *list, ds_list_iter_t it, ds_free_f free_func) {
  // Check input parameters
  if (!list || !it) return NULL;

  ds_list_node_t *ret, *tmp;
  
  // Case 1: element count of list equal to 1
  if (list->size == 1) {
    // Copy
    tmp = list->tail;
    ret = NULL;
    // Update
    list->head = NULL;
    list->tail = NULL;
  }
  // Case 2: it == tail
  else if (it == list->tail) {
    // Copy
    tmp = list->tail;
    ret = NULL;
    // Update
    tmp->prev->next = NULL;
    list->tail = tmp->prev;
  }
  // Case 3: it == head
  else if (it == list->head) {
    // Copy
    tmp = list->head;
    ret = tmp->next;
    // Update
    tmp->next->prev = NULL;
    list->head = tmp->next;
  }
  // Case 4: Otherwise
  else {
    // Copy
    tmp = it;
    ret = tmp->next;
    // Update
    tmp->prev->next = tmp->next;
    tmp->next->prev = tmp->prev;
  }

  // Free
  if (free_func)
    free_func(tmp->data);
  free(tmp);

  list->size --;

  return ret;
}

/**
 * @brief Remove all elements from the list.
 *
 * @param list       Pointer to the list.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_list_clear(ds_list_t *list, ds_free_f free_func) {
  // Check input parameters
  if (!list) return;

  // Traverse each node for free
  ds_list_node_t *p = list->head;
  while (p != NULL) {
    if (free_func)
      free_func(p->data);

    // Copy
    ds_list_node_t *tmp = p;
    // Update
    p = p->next;
    // Free
    free(tmp);
  }

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
}

/**
 * @brief Get an iterator pointing to the first element
 *
 * @param list Pointer to the list.
 *
 * @return Returns NULL (End) if the list is empty.
 */
ds_list_iter_t ds_list_iter_begin(ds_list_t *list) {
  // Check input parameters
  if (!list || list->size == 0) return NULL;

  return list->head;
}

/**
 * @brief Get an iterator pointing to "End" (Sentinel)
 *
 * @param list Pointer to the list.
 *
 * @return Always returns NULL. Used to determine whether traversal has ended.
 */
ds_list_iter_t ds_list_iter_end(ds_list_t *list) {
  (void) list;
  return NULL;
}

/**
 * @brief Get an iterator pointing to the last valid element (Tail)
 *
 * @note This is a helper function to support reverse traversal.
 *
 * @return Returns NULL if the list is empty.
 */
ds_list_iter_t ds_list_iter_tail(ds_list_t *list) {
  // Check input parameters
  if (!list || list->size == 0) return NULL;

  return list->tail;
}

/**
 * @brief Get the next element at the current iterator position and 
 *        move forward by one position.
 *
 * @param Pointer to an iterator of type ds_list_iter_t.
 *
 * @return Pointer to the iterator.
 */
ds_list_iter_t ds_list_iter_next(ds_list_iter_t it) {
  // Check input parameters
  if (!it) return NULL;
  
  return it->next;
}

/**
 * @brief Get the previous element at the current iterator position and 
 *        move backward by one position.
 *
 * @param Pointer to an iterator of type ds_list_iter_t.
 *
 * @return Pointer to the iterator.
 */
ds_list_iter_t ds_list_iter_prev(ds_list_iter_t it) {
  // Check input parameters
  if (!it) return NULL;

  return it->prev;
}

/**
 * @brief Get the data pointed to by the iterator.
 *
 * @param Pointer to an iterator of type ds_list_iter_t.
 *
 * @return Data pointed to by the iterator.
 */
void *ds_list_iter_get(ds_list_iter_t it) {
  // Check input parameters
  if (!it) return NULL;

  return it->data;
}

/**
 * @brief Determine if two iterators are equal.
 *
 * @param Two iterators to be compared.
 *
 * @return Returns true if they are equal, otherwise false.
 */
bool ds_list_iter_equal(const ds_list_iter_t a, const ds_list_iter_t b) {
  // Check input parameters
  if (!a || !b) return false;

  return a == b; 
}
