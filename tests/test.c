/*
** tests/test.c -- A simple test framework.
*/

#include "ds_vector.h"
#include "ds_list.h"
#include "ds_deque.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------ Colored output ------------ */
#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_RESET "\033[0m"

/* ------------ Test statistics ------------ */
static int tests_passed = 0;
static int tests_failed = 0;

/* ------------ Test macros ------------ */
#define ASSERT(cond, msg) \
  do { \
    if (cond) { \
      printf(COLOR_GREEN "[PASS] %s" COLOR_RESET "\n", msg); \
      tests_passed++; \
    } else { \
      printf(COLOR_RED "[FAIL] %s (%s:%d)" COLOR_RESET "\n", msg, __FILE__, __LINE__); \
      tests_failed++; \
    } \
  } while (0)

#define ASSERT_EQ(a, b, msg)     ASSERT((a) == (b), msg)
#define ASSERT_NE(a, b, msg)     ASSERT((a) != (b), msg)
#define ASSERT_NULL(p, msg)      ASSERT((p) == NULL, msg)
#define ASSERT_NOT_NULL(p, msg)  ASSERT((p) != NULL, msg)

#define TEST_FUNC(name) void name()

/* --------- Test function --------- */
typedef void(*test_fn_t)(void);

typedef struct {
  const char *name;
  test_fn_t func;
} test_case_t;

static int run_tests(const test_case_t *tests, size_t num_tests) {
  for (size_t i = 0; i < num_tests; ++ i) {
    printf("Running test: %s\n", tests[i].name);
    tests[i].func();
  }
  printf("\nTotal tests passed: %d, failed: %d\n", tests_passed, tests_failed);
  return tests_failed > 0 ? 1 : 0;
}

/* ------------ Helper for Testing ------------ */
static int *new_int(int v) {
    int *p = malloc(sizeof(int));
    *p = v;
    return p;
}

static int free_count = 0;

static void count_free(void *p) {
    free(p);
    free_count++;
}

/* ------------ Test Function: Vector ------------ */
TEST_FUNC(test_vector) {
    // 1. Creation
    ds_vector_t *vec = ds_vector_create(2); // 小容量以触发扩容
    ASSERT_NOT_NULL(vec, "Vector creation failed");
    ASSERT_EQ(ds_vector_size(vec), 0, "Initial size should be 0");
    ASSERT_EQ(ds_vector_capacity(vec), 2, "Initial capacity check");

    // 2. Push Back & Resize
    ds_vector_push_back(vec, new_int(10));
    ds_vector_push_back(vec, new_int(20));
    ds_vector_push_back(vec, new_int(30)); // Should trigger resize here

    ASSERT_EQ(ds_vector_size(vec), 3, "Size after push");
    ASSERT_EQ(ds_vector_capacity(vec), 4, "Capacity should double");

    // 3. Access (Get)
    int *val0 = (int *)ds_vector_get(vec, 0);
    int *val2 = (int *)ds_vector_get(vec, 2);
    ASSERT_NOT_NULL(val0, "Get index 0");
    ASSERT_EQ(*val0, 10, "Value at index 0");
    ASSERT_EQ(*val2, 30, "Value at index 2");

    // 4. Insert
    ds_vector_insert(vec, 1, new_int(99)); // [10, 99, 20, 30]
    int *val1 = (int *)ds_vector_get(vec, 1);
    ASSERT_EQ(*val1, 99, "Value at inserted index");
    ASSERT_EQ(ds_vector_size(vec), 4, "Size after insert");

    // 5. Set (Replace)
    free_count = 0;
    ds_vector_set(vec, 0, new_int(88), count_free); // Replace 10 with 88
    val0 = (int *)ds_vector_get(vec, 0);
    ASSERT_EQ(*val0, 88, "Value after set");
    ASSERT_EQ(free_count, 1, "Old element should be freed");

    // 6. Remove
    free_count = 0;
    ds_vector_remove(vec, 1, count_free); // Remove 99. Vec: [88, 20, 30]
    ASSERT_EQ(ds_vector_size(vec), 3, "Size after remove");
    ASSERT_EQ(free_count, 1, "Removed element should be freed");
    
    val1 = (int *)ds_vector_get(vec, 1);
    ASSERT_EQ(*val1, 20, "Elements should shift after remove");

    // 7. Pop Back
    int *popped = (int *)ds_vector_pop_back(vec); // Pop 30
    ASSERT_EQ(*popped, 30, "Popped value");
    free(popped); // Manual free since pop transfers ownership

    // 8. Destroy
    free_count = 0;
    ds_vector_destroy(vec, count_free); 
    ASSERT_EQ(free_count, 2, "Destroy should free remaining elements");
}

/* ------------ Test Function: List ------------ */
TEST_FUNC(test_list) {
    // 1. Creation
    ds_list_t *list = ds_list_create();
    ASSERT_NOT_NULL(list, "List creation failed");
    ASSERT_EQ(ds_list_size(list), 0, "Initial size 0");

    // 2. Push Back & Front [10, 20, 30] -> [5, 10, 20, 30]
    ds_list_push_back(list, new_int(10));
    ds_list_push_back(list, new_int(20));
    ds_list_push_back(list, new_int(30));
    ds_list_push_front(list, new_int(5));
    ASSERT_EQ(ds_list_size(list), 4, "Size after pushes");

    // 3. Iteration (Forward)
    ds_list_iter_t it = ds_list_iter_begin(list);
    int *val = (int *)ds_list_iter_get(it);
    ASSERT_EQ(*val, 5, "First element check");
    
    it = ds_list_iter_next(it); // -> 10
    it = ds_list_iter_next(it); // -> 20
    val = (int *)ds_list_iter_get(it);
    ASSERT_EQ(*val, 20, "Third element check");

    // 4. Reverse Iteration (Tail)
    it = ds_list_iter_tail(list);
    val = (int *)ds_list_iter_get(it);
    ASSERT_EQ(*val, 30, "Tail element check");
    
    it = ds_list_iter_prev(it); // -> 20
    val = (int *)ds_list_iter_get(it);
    ASSERT_EQ(*val, 20, "Prev element check");

    // 5. Insert Middle: Insert 15 before 20. List: [5, 10, 15, 20, 30]
    // Current 'it' points to 20
    ds_list_insert(list, it, new_int(15));
    ASSERT_EQ(ds_list_size(list), 5, "Size after insert");
    
    it = ds_list_iter_prev(it); // Should point to new 15
    val = (int *)ds_list_iter_get(it);
    ASSERT_EQ(*val, 15, "Check inserted value");

    // 6. Remove Middle: Remove 15. List: [5, 10, 20, 30]
    free_count = 0;
    it = ds_list_remove(list, it, count_free); 
    // Remove returns iterator to next element (20)
    ASSERT_EQ(free_count, 1, "Removed element freed");
    ASSERT_EQ(ds_list_size(list), 4, "Size after remove");
    
    val = (int *)ds_list_iter_get(it);
    ASSERT_EQ(*val, 20, "Return iterator check");

    // 7. Remove Head/Tail via Pop
    int *head_val = (int *)ds_list_pop_front(list); // Pop 5
    ASSERT_EQ(*head_val, 5, "Pop front check");
    free(head_val);

    int *tail_val = (int *)ds_list_pop_back(list); // Pop 30
    ASSERT_EQ(*tail_val, 30, "Pop back check");
    free(tail_val);

    // 8. Clear
    free_count = 0;
    ds_list_clear(list, count_free);
    ASSERT_EQ(ds_list_size(list), 0, "Size after clear");
    ASSERT_EQ(free_count, 2, "Clear freed remaining elements"); // 10 and 20

    ds_list_destroy(list, NULL);
}

/* ------------ Test Function: Deque ------------ */
TEST_FUNC(test_deque) {
    // 1. Creation
    ds_deque_t *dq = ds_deque_create(4);
    ASSERT_NOT_NULL(dq, "Deque creation failed");
    ASSERT_EQ(ds_deque_size(dq), 0, "Initial size 0");
    ASSERT_EQ(ds_deque_capacity(dq), 4, "Initial capacity check");

    // 2. Basic Push Back/Front
    // State: [2, 1]
    ds_deque_push_back(dq, new_int(1));
    ds_deque_push_front(dq, new_int(2));
    
    int *front = (int *)ds_deque_front(dq);
    int *back = (int *)ds_deque_back(dq);
    ASSERT_EQ(*front, 2, "Front should be 2");
    ASSERT_EQ(*back, 1, "Back should be 1");
    ASSERT_EQ(ds_deque_size(dq), 2, "Size check");

    // 3. Trigger Resize (Capacity 4 -> 8)
    // Push 3, 4, 5. Total 5 elements > 4
    ds_deque_push_back(dq, new_int(3));
    ds_deque_push_back(dq, new_int(4));
    ds_deque_push_back(dq, new_int(5)); 
    // Current logical: [2, 1, 3, 4, 5]
    
    ASSERT_EQ(ds_deque_size(dq), 5, "Size after resize");
    ASSERT_EQ(ds_deque_capacity(dq), 8, "Capacity doubled");

    // Verify order after resize (Linearization check)
    front = (int *)ds_deque_front(dq);
    ASSERT_EQ(*front, 2, "Front preserved after resize");
    
    // 4. Test Wrap-around logic
    // Pop front 2 elements. [3, 4, 5]
    free(ds_deque_pop_front(dq)); // 2
    free(ds_deque_pop_front(dq)); // 1
    
    // Now internal head has moved. 
    // Push back to force tail wrap around if implementation is circular
    ds_deque_push_back(dq, new_int(6));
    ds_deque_push_back(dq, new_int(7));
    ds_deque_push_back(dq, new_int(8));
    // Logical: [3, 4, 5, 6, 7, 8]
    
    front = (int *)ds_deque_front(dq);
    back = (int *)ds_deque_back(dq);
    ASSERT_EQ(*front, 3, "New front is 3");
    ASSERT_EQ(*back, 8, "New back is 8");

    // 5. Mixed Pop
    int *val = (int *)ds_deque_pop_back(dq); // 8
    ASSERT_EQ(*val, 8, "Pop back check");
    free(val);
    
    val = (int *)ds_deque_pop_front(dq); // 3
    ASSERT_EQ(*val, 3, "Pop front check");
    free(val);

    // 6. Clear & Destroy
    free_count = 0;
    ds_deque_destroy(dq, count_free);
    ASSERT_EQ(free_count, 4, "Remaining elements freed"); // 4, 5, 6, 7
}

int main() {
  test_case_t tests[] = {
  {"test_vector", test_vector},
    {"test_list", test_list},
    {"test_deque", test_deque},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
