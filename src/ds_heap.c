#include "ds_heap.h"
#include "ds_common.h"
#include <stddef.h>
#include <stdlib.h>

#define DS_HEAP_DEFAULT_CAPACITY 16
#define DS_HEAP_GROWTH_FACTOR     2

struct ds_heap {
  void **items;         // Dynamic array, store `void *` pointer
  size_t capacity;      // Capacity of current array
  size_t size;          // Element count of current array
  ds_compare_f compare; // Comparison function
};


// Root:        index = 0
// Parent:      (index - 1) / 2
// Left Child:  2 * index + 1
// Right Child: 2 * index + 2
#define HEAP_PARENT(i)   (((i) - 1) >> 1)

#define HEAP_LCHILD(i)   (((i) << 1) + 1)
#define HEAP_RCHILD(i)   (((i) << 1) + 2)

#define HEAP_HAS_LCHILD(i, n)  ((((i) << 1) + 1) < (n))
#define HEAP_HAS_RCHILD(i, n)  ((((i) << 1) + 2) < (n))

/**
 * @brief Perform a shift-up operation to adjust the heap.
 *
 * The root node of the heap has i=0 and no parent.  
 * Prevent i==0 from entering the parent calculation.  
 * The sift‑up loop condition is typically while (i > 0),
 * which structurally ensures safety.
 */
static void shift_up(ds_heap_t *heap) {
  // Check input parameters
  if (!heap || heap->size == 0) return;

  size_t i = heap->size - 1;
  void *x = heap->items[i];

  // Note: If compare(a, b) returns a value less then 0,
  //       it indicates that a is "better" than b
  while (i > 0) {
    size_t p = HEAP_PARENT(i);
    void *parent = heap->items[p];

    if (heap->compare(x, parent) < 0) {
      heap->items[i] = parent;
      i = p;
    } else {
      break;
    }
  }

  heap->items[i] = x;
}

/**
 * @brief Perform a shift-down operation to adjust the heap.
 */
static void shift_down(ds_heap_t *heap) {
  // Check input parameters
  if (!heap || heap->size == 0) return;

  size_t n = heap->size;
  size_t i = 0;
  void *x = heap->items[i];

  for (;;) {
    size_t l = HEAP_LCHILD(i);
    if (l >= n) break;            // No children -> leaf

    size_t best = l;
    size_t r = l + 1;

    // Right children exists
    if (r < n) {
      void *left = heap->items[l];
      void *right = heap->items[r];
      // right is better
      if (heap->compare(right, left) < 0) {
        best = r;
      }
    }

    void *child = heap->items[best];
    // child is better -> move child up
    if (heap->compare(child, x) < 0) {
      heap->items[i] = child;
      i = best;
    } else {
      break;
    }
  }

  heap->items[i] = x;
}

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
ds_heap_t *ds_heap_create(ds_compare_f compare, size_t capacity_hint) {
  // Check input param
  if (!compare) return NULL;

  size_t capacity = capacity_hint == 0 ?
                    DS_HEAP_DEFAULT_CAPACITY :
                    capacity_hint;

  // Allocate heap data structure
  ds_heap_t *heap = malloc(sizeof(ds_heap_t));
  if (!heap)
    return NULL;

  // Allocate dynamic array
  heap->items = malloc(sizeof(void *) * capacity);
  if (!heap->items) {
    free(heap);
    return NULL;
  }
  
  heap->size = 0;
  heap->capacity = capacity;
  heap->compare = compare;

  return heap;
}

/**
 * @brief Destroy a heap and optionally free its elements.
 *
 * @param heap       Pointer to the heap.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the vector itself is freed.
 */
void ds_heap_destroy(ds_heap_t *heap, ds_free_f free_func) {
  // Check input param
  if (!heap) return;

  // Traverse each array node for free
  if (free_func) {
    for (size_t i = 0; i < heap->size; ++ i) {
      free_func(heap->items[i]);
    }
  }

  free(heap->items);
  free(heap);
  return;
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_heap_size(const ds_heap_t *heap) {
  if (!heap) return 0;

  return heap->size;
}

/**
 * @brief Get the currently allocated capacity.
 */
size_t ds_heap_capacity(const ds_heap_t *heap) {
  if (!heap) return 0;

  return heap->capacity;
}

/**
 * @brief Check if the heap is empty.
 */
bool ds_heap_is_empty(const ds_heap_t *heap) {
  if (!heap) return true;

  return heap->size == 0;
}

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
ds_status_t ds_heap_push(ds_heap_t *heap, void *element) {
  if (!heap) return DS_ERR_NULL;
  if (!element) return DS_ERR_ARG;

  // Check capacity of the heap whether full
  if (heap->size == heap->capacity) {
    // Expand capacity
    size_t new_cap = heap->capacity * DS_HEAP_GROWTH_FACTOR;
    void **new_arr = realloc(heap->items, sizeof(void *) * new_cap);
    if (!new_arr) return DS_ERR_MEM;

    heap->items = new_arr;
    heap->capacity = new_cap;
  }

  // Insert the new element at the end of the heap
  heap->items[heap->size ++] = element;

  // Shift up
  shift_up(heap);

  return DS_OK;
}

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
void *ds_heap_pop(ds_heap_t *heap) {
  // Check input parameters
  if (!heap || heap->size == 0) return NULL;

  // Store the top element
  void *ret = heap->items[0];

  heap->items[0] = heap->items[heap->size - 1];
  heap->size --;

  // Shift down 
  shift_down(heap);

  return ret;
}

/**
 * @brief Get the top element of the heap.
 *
 * The user should not free this memory.
 *
 * @param heap  Pointer to the heap.
 *
 * @return Pointer to the removed element, or NULL if the heap is empty.
 */
void *ds_heap_top(ds_heap_t *heap) {
  // Check input parameters
  if (!heap || heap->size == 0) return NULL;
  
  return heap->items[0];
}

/**
 * @brief Remove all elements from the heap.
 *
 * @param heap       Pointer to the heap.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_heap_clear(ds_heap_t *heap, ds_free_f free_func) {
  // Check input parameters
  if (!heap || heap->size == 0) return;

  if (free_func) {
    for (size_t i = 0; i < heap->size; ++ i) {
      free_func(heap->items[i]);
    }
  }

  heap->size = 0;
  return;
}
