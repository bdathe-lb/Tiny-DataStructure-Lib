#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ds_stack.h"
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

TEST_FUNC(test_stack_create_destroy_basic) {
  ds_stack_t *s = ds_stack_create(0);
  ASSERT_NOT_NULL(s, "create(0) non-NULL");
  ASSERT_EQ(ds_stack_size(s), 0u, "new stack size == 0");
  ASSERT(ds_stack_is_empty(s) == true, "new stack empty");
  ASSERT(ds_stack_capacity(s) >= 1u, "capacity >= 1");

  ds_stack_destroy(s, NULL);
  ASSERT(1, "destroy ok");

  ds_stack_destroy(NULL, NULL);
  ASSERT(1, "destroy(NULL) ok");
}

TEST_FUNC(test_stack_null_safety) {
  ASSERT_EQ(ds_stack_size(NULL), 0u, "size(NULL)==0");
  ASSERT_EQ(ds_stack_capacity(NULL), 0u, "capacity(NULL)==0");
  ASSERT(ds_stack_is_empty(NULL) == true, "is_empty(NULL)==true");
  ASSERT_NULL(ds_stack_pop(NULL), "pop(NULL)==NULL");
  ASSERT_NULL(ds_stack_top(NULL), "top(NULL)==NULL");
  ds_stack_clear(NULL, NULL);
  ASSERT(1, "clear(NULL) ok");
}

TEST_FUNC(test_stack_push_pop_top_lifo) {
  ds_stack_t *s = ds_stack_create(2);
  ASSERT_NOT_NULL(s, "create(2)");

  ASSERT_NULL(ds_stack_top(s), "top(empty)==NULL");
  ASSERT_NULL(ds_stack_pop(s), "pop(empty)==NULL");

  ASSERT_EQ(ds_stack_push(s, mk_int(1)), DS_OK, "push 1");
  ASSERT_EQ(int_val(ds_stack_top(s)), 1, "top==1");
  ASSERT_EQ(ds_stack_size(s), 1u, "size==1");

  ASSERT_EQ(ds_stack_push(s, mk_int(2)), DS_OK, "push 2");
  ASSERT_EQ(int_val(ds_stack_top(s)), 2, "top==2");
  ASSERT_EQ(ds_stack_size(s), 2u, "size==2");

  ASSERT_EQ(ds_stack_push(s, mk_int(3)), DS_OK, "push 3 (grow)");
  ASSERT_EQ(int_val(ds_stack_top(s)), 3, "top==3");
  ASSERT_EQ(ds_stack_size(s), 3u, "size==3");

  int *p = (int *)ds_stack_pop(s);
  ASSERT_NOT_NULL(p, "pop non-NULL");
  ASSERT_EQ(*p, 3, "pop==3");
  free(p);

  p = (int *)ds_stack_pop(s);
  ASSERT_NOT_NULL(p, "pop non-NULL");
  ASSERT_EQ(*p, 2, "pop==2");
  free(p);

  p = (int *)ds_stack_pop(s);
  ASSERT_NOT_NULL(p, "pop non-NULL");
  ASSERT_EQ(*p, 1, "pop==1");
  free(p);

  ASSERT_EQ(ds_stack_size(s), 0u, "size==0 after pops");
  ASSERT(ds_stack_is_empty(s) == true, "empty after pops");
  ASSERT_NULL(ds_stack_pop(s), "pop on empty==NULL");
  ASSERT_NULL(ds_stack_top(s), "top on empty==NULL");

  ds_stack_destroy(s, counted_free);
}

TEST_FUNC(test_stack_clear_frees_elements) {
  ds_stack_t *s = ds_stack_create(0);
  ASSERT_NOT_NULL(s, "create default");

  for (int i = 0; i < 10; i++) {
    ASSERT_EQ(ds_stack_push(s, mk_int(i)), DS_OK, "push i");
  }
  ASSERT_EQ(ds_stack_size(s), 10u, "size==10");

  int free_before = g_free_count;
  ds_stack_clear(s, counted_free);
  ASSERT_EQ(g_free_count, free_before + 10, "clear frees 10");
  ASSERT_EQ(ds_stack_size(s), 0u, "size==0 after clear");
  ASSERT(ds_stack_is_empty(s) == true, "empty after clear");
  ASSERT_NULL(ds_stack_top(s), "top NULL after clear");

  ds_stack_destroy(s, counted_free);
}

TEST_FUNC(test_stack_bulk_stress) {
  ds_stack_t *s = ds_stack_create(1);
  ASSERT_NOT_NULL(s, "create(1)");

  const int N = 20000;
  for (int i = 0; i < N; i++) {
    ds_status_t st = ds_stack_push(s, mk_int(i));
    ASSERT_EQ(st, DS_OK, "bulk push DS_OK");
  }
  ASSERT_EQ(ds_stack_size(s), (size_t)N, "bulk size==N");
  ASSERT(ds_stack_capacity(s) >= (size_t)N, "capacity>=N");

  for (int i = N - 1; i >= 0; i--) {
    int *p = (int *)ds_stack_pop(s);
    ASSERT_NOT_NULL(p, "bulk pop non-NULL");
    ASSERT_EQ(*p, i, "bulk pop order LIFO");
    free(p);
  }
  ASSERT_EQ(ds_stack_size(s), 0u, "size==0 after bulk pops");

  ds_stack_destroy(s, counted_free);
}

TEST_FUNC(test_stack_error_codes_push) {
  ds_stack_t *s = ds_stack_create(0);
  ASSERT_NOT_NULL(s, "create");

  int *p = mk_int(123);
  ASSERT_NOT_NULL(p, "mk_int");

  /* 这里按你之前 vector/deque 的规范断言：
     - stack==NULL => DS_ERR_NULL
     - element==NULL => DS_ERR_ARG
     你当前实现会返回 DS_ERR_BOUNDS，所以会 FAIL，逼你修 ds_stack_push。 */
  ASSERT_EQ(ds_stack_push(NULL, p), DS_ERR_NULL, "push(NULL, elem) => DS_ERR_NULL");
  ASSERT_EQ(ds_stack_push(s, NULL), DS_ERR_ARG, "push(stack, NULL) => DS_ERR_ARG");

  free(p);
  ds_stack_destroy(s, NULL);
}

/* ===================== main ===================== */

int main() {
  test_case_t tests[] = {
    {"stack_create_destroy_basic", test_stack_create_destroy_basic},
    {"stack_null_safety", test_stack_null_safety},
    {"stack_push_pop_top_lifo", test_stack_push_pop_top_lifo},
    {"stack_clear_frees_elements", test_stack_clear_frees_elements},
    {"stack_bulk_stress", test_stack_bulk_stress},
    {"stack_error_codes_push", test_stack_error_codes_push},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
