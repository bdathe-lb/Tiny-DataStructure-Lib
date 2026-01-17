#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ds_queue.h"
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

/* ===================== Tests ===================== */

TEST_FUNC(test_queue_create_destroy_basic) {
  ds_queue_t *q = ds_queue_create(0);
  ASSERT_NOT_NULL(q, "create(0) non-NULL");
  ASSERT_EQ(ds_queue_size(q), 0u, "new queue size==0");
  ASSERT(ds_queue_is_empty(q) == true, "new queue empty");
  ASSERT(ds_queue_capacity(q) >= 1u, "capacity>=1");

  ds_queue_destroy(q, NULL);
  ASSERT(1, "destroy ok");

  ds_queue_destroy(NULL, NULL);
  ASSERT(1, "destroy(NULL) ok");
}

TEST_FUNC(test_queue_null_safety) {
  ASSERT_EQ(ds_queue_size(NULL), 0u, "size(NULL)==0");
  ASSERT_EQ(ds_queue_capacity(NULL), 0u, "capacity(NULL)==0");
  ASSERT(ds_queue_is_empty(NULL) == true, "is_empty(NULL)==true");

  int *p = mk_int(1);
  ASSERT_NOT_NULL(p, "mk_int");
  ASSERT_EQ(ds_queue_push(NULL, p), DS_ERR_NULL, "push(NULL,elem)==DS_ERR_NULL");
  free(p);

  ASSERT_NULL(ds_queue_pop(NULL), "pop(NULL)==NULL");
  ASSERT_NULL(ds_queue_front(NULL), "front(NULL)==NULL");

  ds_queue_clear(NULL, NULL);
  ASSERT(1, "clear(NULL) ok");
}

TEST_FUNC(test_queue_arg_checks) {
  ds_queue_t *q = ds_queue_create(2);
  ASSERT_NOT_NULL(q, "create(2)");

  ASSERT_EQ(ds_queue_push(q, NULL), DS_ERR_ARG, "push(NULL elem)==DS_ERR_ARG");

  ds_queue_destroy(q, NULL);
}

TEST_FUNC(test_queue_fifo_basic) {
  ds_queue_t *q = ds_queue_create(4);
  ASSERT_NOT_NULL(q, "create(4)");

  ASSERT_NULL(ds_queue_front(q), "front(empty)==NULL");
  ASSERT_NULL(ds_queue_pop(q), "pop(empty)==NULL");

  ASSERT_EQ(ds_queue_push(q, mk_int(1)), DS_OK, "push 1");
  ASSERT_EQ(int_val(ds_queue_front(q)), 1, "front==1");
  ASSERT_EQ(ds_queue_size(q), 1u, "size==1");

  ASSERT_EQ(ds_queue_push(q, mk_int(2)), DS_OK, "push 2");
  ASSERT_EQ(int_val(ds_queue_front(q)), 1, "front still==1");
  ASSERT_EQ(ds_queue_size(q), 2u, "size==2");

  ASSERT_EQ(ds_queue_push(q, mk_int(3)), DS_OK, "push 3");
  ASSERT_EQ(ds_queue_size(q), 3u, "size==3");

  int *p = (int *)ds_queue_pop(q);
  ASSERT_NOT_NULL(p, "pop non-NULL");
  ASSERT_EQ(*p, 1, "pop==1");
  free(p);

  p = (int *)ds_queue_pop(q);
  ASSERT_NOT_NULL(p, "pop non-NULL");
  ASSERT_EQ(*p, 2, "pop==2");
  free(p);

  p = (int *)ds_queue_pop(q);
  ASSERT_NOT_NULL(p, "pop non-NULL");
  ASSERT_EQ(*p, 3, "pop==3");
  free(p);

  ASSERT_EQ(ds_queue_size(q), 0u, "size==0 after pops");
  ASSERT(ds_queue_is_empty(q) == true, "empty after pops");
  ASSERT_NULL(ds_queue_front(q), "front==NULL when empty");
  ASSERT_NULL(ds_queue_pop(q), "pop==NULL when empty");

  ds_queue_destroy(q, counted_free);
}

TEST_FUNC(test_queue_clear_frees) {
  ds_queue_t *q = ds_queue_create(0);
  ASSERT_NOT_NULL(q, "create default");

  for (int i = 0; i < 10; i++) {
    ASSERT_EQ(ds_queue_push(q, mk_int(i)), DS_OK, "push i");
  }
  ASSERT_EQ(ds_queue_size(q), 10u, "size==10");

  int free_before = g_free_count;
  ds_queue_clear(q, counted_free);
  ASSERT_EQ(g_free_count, free_before + 10, "clear frees 10");
  ASSERT_EQ(ds_queue_size(q), 0u, "size==0 after clear");
  ASSERT(ds_queue_is_empty(q) == true, "empty after clear");
  ASSERT_NULL(ds_queue_front(q), "front NULL after clear");

  ds_queue_destroy(q, counted_free);
}

TEST_FUNC(test_queue_bulk_stress) {
  ds_queue_t *q = ds_queue_create(1);
  ASSERT_NOT_NULL(q, "create(1)");

  const int N = 20000;
  for (int i = 0; i < N; i++) {
    ds_status_t st = ds_queue_push(q, mk_int(i));
    ASSERT_EQ(st, DS_OK, "bulk push DS_OK");
  }
  ASSERT_EQ(ds_queue_size(q), (size_t)N, "bulk size==N");
  ASSERT(ds_queue_capacity(q) >= (size_t)N, "capacity>=N");

  for (int i = 0; i < N; i++) {
    int *p = (int *)ds_queue_pop(q);
    ASSERT_NOT_NULL(p, "bulk pop non-NULL");
    ASSERT_EQ(*p, i, "bulk pop FIFO order");
    free(p);
  }
  ASSERT_EQ(ds_queue_size(q), 0u, "size==0 after bulk pops");

  ds_queue_destroy(q, counted_free);
}

/* ===================== main ===================== */

int main() {
  test_case_t tests[] = {
    {"queue_create_destroy_basic", test_queue_create_destroy_basic},
    {"queue_null_safety", test_queue_null_safety},
    {"queue_arg_checks", test_queue_arg_checks},
    {"queue_fifo_basic", test_queue_fifo_basic},
    {"queue_clear_frees", test_queue_clear_frees},
    {"queue_bulk_stress", test_queue_bulk_stress},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
