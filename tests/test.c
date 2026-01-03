/*
** tests/test.c -- A simple test framework.
*/

#include "ds_vector.h"
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

/* ------------ Test Function ------------ */
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

int main() {
  test_case_t tests[] = {
  {"test_vector", test_vector},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
