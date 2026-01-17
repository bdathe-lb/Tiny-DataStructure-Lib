#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ds_heap.h"
#include "ds_common.h"

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

/* ===================== Helpers ===================== */

static int *mk_int(int v) {
  int *p = (int *)malloc(sizeof(int));
  if (!p) return NULL;
  *p = v;
  return p;
}

static int int_val(void *p) {
  return p ? *(int *)p : 0;
}

static int g_free_count = 0;
static void counted_free(void *p) {
  if (p) g_free_count++;
  free(p);
}

/* compare: return <0 means a is "better" */
static int int_compare_min(const void *a, const void *b) {
  int x = *(const int *)a;
  int y = *(const int *)b;
  if (x < y) return -1;
  if (x > y) return 1;
  return 0;
}

static int int_compare_max(const void *a, const void *b) {
  int x = *(const int *)a;
  int y = *(const int *)b;
  if (x > y) return -1;  /* larger is "better" */
  if (x < y) return 1;
  return 0;
}

/* ===================== Tests ===================== */

TEST_FUNC(test_heap_create_destroy_basic) {
  ds_heap_t *h = ds_heap_create(int_compare_min, 0);
  ASSERT_NOT_NULL(h, "create(min,0) non-NULL");
  ASSERT_EQ(ds_heap_size(h), 0u, "new heap size==0");
  ASSERT(ds_heap_is_empty(h) == true, "new heap empty");
  ASSERT(ds_heap_capacity(h) >= 1u, "capacity>=1");

  ds_heap_destroy(h, NULL);
  ASSERT(1, "destroy ok");

  ds_heap_destroy(NULL, NULL);
  ASSERT(1, "destroy(NULL) ok");
}

TEST_FUNC(test_heap_create_null_compare) {
  ds_heap_t *h = ds_heap_create(NULL, 0);
  ASSERT_NULL(h, "create(NULL compare) returns NULL");
}

TEST_FUNC(test_heap_null_safety_and_error_codes) {
  ASSERT_EQ(ds_heap_size(NULL), 0u, "size(NULL)==0");
  ASSERT_EQ(ds_heap_capacity(NULL), 0u, "capacity(NULL)==0");
  ASSERT(ds_heap_is_empty(NULL) == true, "is_empty(NULL)==true");
  ASSERT_NULL(ds_heap_top(NULL), "top(NULL)==NULL");
  ASSERT_NULL(ds_heap_pop(NULL), "pop(NULL)==NULL");
  ds_heap_clear(NULL, NULL);
  ASSERT(1, "clear(NULL) ok");

  int *p = mk_int(1);
  ASSERT_NOT_NULL(p, "mk_int");
  ASSERT_EQ(ds_heap_push(NULL, p), DS_ERR_NULL, "push(NULL,elem)==DS_ERR_NULL");
  free(p);
}

TEST_FUNC(test_heap_push_pop_min_order) {
  ds_heap_t *h = ds_heap_create(int_compare_min, 2);
  ASSERT_NOT_NULL(h, "create(min,2)");

  ASSERT_NULL(ds_heap_top(h), "top(empty)==NULL");
  ASSERT_NULL(ds_heap_pop(h), "pop(empty)==NULL");

  int vals[] = {50, 10, 30, 5, 20, 20};
  for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
    ASSERT_EQ(ds_heap_push(h, mk_int(vals[i])), DS_OK, "push OK");
  }
  ASSERT_EQ(ds_heap_size(h), 6u, "size==6");

  /* min-heap => pop should be non-decreasing */
  int last = -2147483648; /* INT_MIN */
  for (int i = 0; i < 6; i++) {
    int *p = (int *)ds_heap_pop(h);
    ASSERT_NOT_NULL(p, "pop non-NULL");
    ASSERT(*p >= last, "min-heap pop is non-decreasing");
    last = *p;
    free(p);
  }
  ASSERT_EQ(ds_heap_size(h), 0u, "size==0 after pops");
  ASSERT(ds_heap_is_empty(h) == true, "empty after pops");

  ds_heap_destroy(h, counted_free);
}

TEST_FUNC(test_heap_push_pop_max_order) {
  ds_heap_t *h = ds_heap_create(int_compare_max, 1);
  ASSERT_NOT_NULL(h, "create(max,1)");

  int vals[] = {7, 1, 9, 9, 3, 5};
  for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
    ASSERT_EQ(ds_heap_push(h, mk_int(vals[i])), DS_OK, "push OK");
  }
  ASSERT_EQ(ds_heap_size(h), 6u, "size==6");

  /* max-heap => pop should be non-increasing */
  int last = 2147483647; /* INT_MAX */
  for (int i = 0; i < 6; i++) {
    int *p = (int *)ds_heap_pop(h);
    ASSERT_NOT_NULL(p, "pop non-NULL");
    ASSERT(*p <= last, "max-heap pop is non-increasing");
    last = *p;
    free(p);
  }
  ASSERT_EQ(ds_heap_size(h), 0u, "empty after pops");

  ds_heap_destroy(h, counted_free);
}

TEST_FUNC(test_heap_top_peek_does_not_remove) {
  ds_heap_t *h = ds_heap_create(int_compare_min, 0);
  ASSERT_NOT_NULL(h, "create");

  ds_heap_push(h, mk_int(3));
  ds_heap_push(h, mk_int(1));
  ds_heap_push(h, mk_int(2));

  int *t = (int *)ds_heap_top(h);
  ASSERT_NOT_NULL(t, "top non-NULL");
  ASSERT_EQ(*t, 1, "top == min element");
  ASSERT_EQ(ds_heap_size(h), 3u, "top does not change size");

  int *p = (int *)ds_heap_pop(h);
  ASSERT_EQ(*p, 1, "pop == min element");
  free(p);

  ASSERT_EQ(ds_heap_size(h), 2u, "size==2 after pop");

  ds_heap_destroy(h, counted_free);
}

TEST_FUNC(test_heap_clear_frees) {
  ds_heap_t *h = ds_heap_create(int_compare_min, 0);
  ASSERT_NOT_NULL(h, "create");

  for (int i = 0; i < 100; i++) ds_heap_push(h, mk_int(i));
  ASSERT_EQ(ds_heap_size(h), 100u, "size==100");

  int free_before = g_free_count;
  ds_heap_clear(h, counted_free);
  ASSERT_EQ(g_free_count, free_before + 100, "clear frees 100");
  ASSERT_EQ(ds_heap_size(h), 0u, "size==0 after clear");
  ASSERT(ds_heap_is_empty(h) == true, "empty after clear");
  ASSERT_NULL(ds_heap_top(h), "top NULL after clear");

  ds_heap_destroy(h, counted_free);
}

TEST_FUNC(test_heap_bulk_random_sorted_by_popping) {
  ds_heap_t *h = ds_heap_create(int_compare_min, 4);
  ASSERT_NOT_NULL(h, "create");

  /* fixed seed */
  uint32_t seed = 0xC0FFEEu;
  const int N = 20000;

  for (int i = 0; i < N; i++) {
    seed = seed * 1664525u + 1013904223u;
    int v = (int)(seed & 0x7fffffff);
    ASSERT_EQ(ds_heap_push(h, mk_int(v)), DS_OK, "bulk push OK");
  }
  ASSERT_EQ(ds_heap_size(h), (size_t)N, "size==N");

  int last = -2147483648;
  for (int i = 0; i < N; i++) {
    int *p = (int *)ds_heap_pop(h);
    ASSERT_NOT_NULL(p, "bulk pop non-NULL");
    ASSERT(*p >= last, "popped sequence is sorted non-decreasing");
    last = *p;
    free(p);
  }
  ASSERT_EQ(ds_heap_size(h), 0u, "size==0 after bulk pops");

  ds_heap_destroy(h, counted_free);
}

TEST_FUNC(test_heap_push_arg_checks) {
  ds_heap_t *h = ds_heap_create(int_compare_min, 0);
  ASSERT_NOT_NULL(h, "create");

  ASSERT_EQ(ds_heap_push(h, NULL), DS_ERR_ARG, "push(NULL element)==DS_ERR_ARG");

  ds_heap_destroy(h, counted_free);
}

/* ===================== main ===================== */

int main() {
  test_case_t tests[] = {
    {"heap_create_destroy_basic", test_heap_create_destroy_basic},
    {"heap_create_null_compare", test_heap_create_null_compare},
    {"heap_null_safety_and_error_codes", test_heap_null_safety_and_error_codes},
    {"heap_push_pop_min_order", test_heap_push_pop_min_order},
    {"heap_push_pop_max_order", test_heap_push_pop_max_order},
    {"heap_top_peek_does_not_remove", test_heap_top_peek_does_not_remove},
    {"heap_clear_frees", test_heap_clear_frees},
    {"heap_bulk_random_sorted_by_popping", test_heap_bulk_random_sorted_by_popping},
    {"heap_push_arg_checks", test_heap_push_arg_checks},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
