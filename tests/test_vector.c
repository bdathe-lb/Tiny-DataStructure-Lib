/*
** tests/test.c -- A simple test framework.
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ds_common.h"
#include "ds_vector.h"

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

static int g_free_count = 0;

static void counted_free(void *p) {
  if (p) g_free_count++;
  free(p);
}

static int int_val(void *p) {
  return p ? *(int *)p : 0;
}

/* ===================== Tests ===================== */

TEST_FUNC(test_vector_create_destroy_basic) {
  ds_vector_t *v = ds_vector_create(0);
  ASSERT_NOT_NULL(v, "create(0) returns non-NULL");
  ASSERT_EQ(ds_vector_size(v), 0u, "new vector size == 0");
  ASSERT(ds_vector_is_empty(v) == true, "new vector is empty");
  ASSERT(ds_vector_capacity(v) >= 1u, "new vector capacity >= 1");
  ds_vector_destroy(v, NULL);
  ASSERT(1, "destroy(NULL free_func) does not crash");
}

TEST_FUNC(test_vector_null_safety_queries) {
  ASSERT_EQ(ds_vector_size(NULL), 0u, "size(NULL) == 0");
  ASSERT_EQ(ds_vector_capacity(NULL), 0u, "capacity(NULL) == 0");
  ASSERT(ds_vector_is_empty(NULL) == true, "is_empty(NULL) == true");
  ASSERT_NULL(ds_vector_get(NULL, 0), "get(NULL, 0) == NULL");
  ASSERT_NULL(ds_vector_pop_back(NULL), "pop_back(NULL) == NULL");
  ds_vector_clear(NULL, NULL);
  ASSERT(1, "clear(NULL) does not crash");
  ds_vector_destroy(NULL, NULL);
  ASSERT(1, "destroy(NULL) does not crash");
}

TEST_FUNC(test_vector_error_codes_null_and_arg) {
  ds_vector_t *v = ds_vector_create(2);
  ASSERT_NOT_NULL(v, "create(2) returns non-NULL");

  /* reserve */
  ASSERT_EQ(ds_vector_reserve(NULL, 10), DS_ERR_NULL, "reserve(NULL, 10) == DS_ERR_NULL");

  /* push_back */
  int *p = mk_int(123);
  ASSERT_NOT_NULL(p, "mk_int ok");
  ASSERT_EQ(ds_vector_push_back(NULL, p), DS_ERR_NULL, "push_back(NULL, elem) == DS_ERR_NULL");
  ASSERT_EQ(ds_vector_push_back(v, NULL), DS_ERR_ARG, "push_back(vec, NULL) == DS_ERR_ARG");

  /* insert */
  ASSERT_EQ(ds_vector_insert(NULL, 0, p), DS_ERR_NULL, "insert(NULL, 0, elem) == DS_ERR_NULL");
  ASSERT_EQ(ds_vector_insert(v, 0, NULL), DS_ERR_ARG, "insert(vec, 0, NULL) == DS_ERR_ARG");

  /* set */
  ASSERT_EQ(ds_vector_set(NULL, 0, p, NULL), DS_ERR_NULL, "set(NULL, ...) == DS_ERR_NULL");
  ASSERT_EQ(ds_vector_set(v, 0, NULL, NULL), DS_ERR_BOUNDS, "set on empty -> DS_ERR_BOUNDS (index>=size)");

  free(p);
  ds_vector_destroy(v, NULL);
}

TEST_FUNC(test_vector_push_get_pop) {
  ds_vector_t *v = ds_vector_create(2);
  ASSERT_NOT_NULL(v, "create(2) returns non-NULL");

  int *a = mk_int(10);
  int *b = mk_int(20);
  ASSERT_NOT_NULL(a, "mk_int(10)");
  ASSERT_NOT_NULL(b, "mk_int(20)");

  ASSERT_EQ(ds_vector_push_back(v, a), DS_OK, "push_back a == DS_OK");
  ASSERT_EQ(ds_vector_push_back(v, b), DS_OK, "push_back b == DS_OK");

  ASSERT_EQ(ds_vector_size(v), 2u, "size after 2 push_back == 2");
  ASSERT(ds_vector_is_empty(v) == false, "vector not empty");

  ASSERT_EQ(int_val(ds_vector_get(v, 0)), 10, "get(0) == 10");
  ASSERT_EQ(int_val(ds_vector_get(v, 1)), 20, "get(1) == 20");
  ASSERT_NULL(ds_vector_get(v, 2), "get(out of range) == NULL");

  void *x = ds_vector_pop_back(v);
  ASSERT_NOT_NULL(x, "pop_back returns non-NULL");
  ASSERT_EQ(int_val(x), 20, "pop_back == 20");
  free(x);

  x = ds_vector_pop_back(v);
  ASSERT_NOT_NULL(x, "pop_back second returns non-NULL");
  ASSERT_EQ(int_val(x), 10, "pop_back == 10");
  free(x);

  ASSERT_EQ(ds_vector_size(v), 0u, "size after pops == 0");
  ASSERT(ds_vector_is_empty(v) == true, "vector empty after pops");
  ASSERT_NULL(ds_vector_pop_back(v), "pop_back on empty == NULL");

  ds_vector_destroy(v, NULL);
}

TEST_FUNC(test_vector_insert_middle_and_order) {
  ds_vector_t *v = ds_vector_create(0);
  ASSERT_NOT_NULL(v, "create default");

  int *a = mk_int(1);
  int *b = mk_int(2);
  int *c = mk_int(3);

  ASSERT_EQ(ds_vector_push_back(v, a), DS_OK, "push_back 1");
  ASSERT_EQ(ds_vector_push_back(v, c), DS_OK, "push_back 3");
  ASSERT_EQ(ds_vector_insert(v, 1, b), DS_OK, "insert 2 at index 1");

  ASSERT_EQ(ds_vector_size(v), 3u, "size == 3 after insert");
  ASSERT_EQ(int_val(ds_vector_get(v, 0)), 1, "order[0] == 1");
  ASSERT_EQ(int_val(ds_vector_get(v, 1)), 2, "order[1] == 2");
  ASSERT_EQ(int_val(ds_vector_get(v, 2)), 3, "order[2] == 3");

  /* insert out of bounds: index > size */
  /* 上面 mk_int(99) 成功了的话会泄漏，这里把它处理掉： */
  /* 由于 insert 失败时不会接管所有权，我们需要手动 free */
  /* 但我们拿不到指针了，所以不直接这么写。改成先保存指针： */
  /* —— 为避免这类问题，下面用安全写法： */
  int *tmp = mk_int(99);
  ASSERT_NOT_NULL(tmp, "mk_int(99)");
  ASSERT_EQ(ds_vector_insert(v, 4, tmp), DS_ERR_BOUNDS, "insert tmp index>size => DS_ERR_BOUNDS");
  free(tmp);

  ds_vector_destroy(v, counted_free);
  ASSERT_EQ(g_free_count, 3, "destroy with free_func frees 3 elements");
}

TEST_FUNC(test_vector_reserve_behavior) {
  ds_vector_t *v = ds_vector_create(2);
  ASSERT_NOT_NULL(v, "create(2)");

  ASSERT_EQ(ds_vector_reserve(v, 1), DS_OK, "reserve(1) on empty but cap>=2 => DS_OK");
  size_t old_cap = ds_vector_capacity(v);
  ASSERT(old_cap >= 2u, "old_cap >= 2");

  int *a = mk_int(11);
  int *b = mk_int(22);
  ds_vector_push_back(v, a);
  ds_vector_push_back(v, b);

  /* new_capacity < size => DS_ERR_BOUNDS */
  ASSERT_EQ(ds_vector_reserve(v, 1), DS_ERR_BOUNDS, "reserve(<size) == DS_ERR_BOUNDS");

  /* grow */
  ASSERT_EQ(ds_vector_reserve(v, old_cap + 10), DS_OK, "reserve(grow) == DS_OK");
  ASSERT(ds_vector_capacity(v) >= old_cap + 10, "capacity >= requested after grow");

  ds_vector_destroy(v, counted_free);
  ASSERT_EQ(g_free_count, 5, "free_count accumulates (prev 3 + this 2 = 5)");
}

TEST_FUNC(test_vector_set_replaces_and_frees_old) {
  ds_vector_t *v = ds_vector_create(0);
  ASSERT_NOT_NULL(v, "create default");

  int *a = mk_int(100);
  int *b = mk_int(200);
  ASSERT_EQ(ds_vector_push_back(v, a), DS_OK, "push_back 100");

  int free_before = g_free_count;
  ASSERT_EQ(ds_vector_set(v, 0, b, counted_free), DS_OK, "set index 0 to 200, free old");
  ASSERT_EQ(g_free_count, free_before + 1, "set freed old exactly once");
  ASSERT_EQ(int_val(ds_vector_get(v, 0)), 200, "get(0) == 200 after set");

  /* element==NULL => DS_ERR_ARG */
  ASSERT_EQ(ds_vector_set(v, 0, NULL, NULL), DS_ERR_ARG, "set element NULL => DS_ERR_ARG");

  /* index out of range => DS_ERR_BOUNDS */
  int *c = mk_int(300);
  ASSERT_NOT_NULL(c, "mk_int(300)");
  ASSERT_EQ(ds_vector_set(v, 99, c, NULL), DS_ERR_BOUNDS, "set OOB => DS_ERR_BOUNDS");
  free(c);

  ds_vector_destroy(v, counted_free);
}

TEST_FUNC(test_vector_remove_and_shift) {
  ds_vector_t *v = ds_vector_create(0);
  ASSERT_NOT_NULL(v, "create default");

  int *a = mk_int(1);
  int *b = mk_int(2);
  int *c = mk_int(3);
  int *d = mk_int(4);

  ds_vector_push_back(v, a);
  ds_vector_push_back(v, b);
  ds_vector_push_back(v, c);
  ds_vector_push_back(v, d);

  ASSERT_EQ(ds_vector_size(v), 4u, "size == 4");

  int free_before = g_free_count;
  ASSERT_EQ(ds_vector_remove(v, 1, counted_free), DS_OK, "remove index 1 (value 2) with free");
  ASSERT_EQ(g_free_count, free_before + 1, "remove freed removed element once");

  ASSERT_EQ(ds_vector_size(v), 3u, "size == 3 after remove");
  ASSERT_EQ(int_val(ds_vector_get(v, 0)), 1, "after remove, [0]==1");
  ASSERT_EQ(int_val(ds_vector_get(v, 1)), 3, "after remove, [1]==3");
  ASSERT_EQ(int_val(ds_vector_get(v, 2)), 4, "after remove, [2]==4");

  ASSERT_EQ(ds_vector_remove(NULL, 0, NULL), DS_ERR_NULL, "remove(NULL,...) => DS_ERR_NULL");
  ASSERT_EQ(ds_vector_remove(v, 99, NULL), DS_ERR_BOUNDS, "remove OOB => DS_ERR_BOUNDS");

  ds_vector_destroy(v, counted_free);
}

TEST_FUNC(test_vector_clear_frees_all_and_keeps_capacity) {
  ds_vector_t *v = ds_vector_create(4);
  ASSERT_NOT_NULL(v, "create(4)");

  size_t cap_before = ds_vector_capacity(v);
  ASSERT(cap_before >= 4u, "cap_before >= 4");

  ds_vector_push_back(v, mk_int(10));
  ds_vector_push_back(v, mk_int(20));
  ds_vector_push_back(v, mk_int(30));
  ASSERT_EQ(ds_vector_size(v), 3u, "size == 3");

  int free_before = g_free_count;
  ds_vector_clear(v, counted_free);
  ASSERT_EQ(ds_vector_size(v), 0u, "size == 0 after clear");
  ASSERT(ds_vector_is_empty(v) == true, "empty after clear");
  ASSERT_EQ(g_free_count, free_before + 3, "clear freed all 3 elements");
  ASSERT_EQ(ds_vector_capacity(v), cap_before, "capacity unchanged after clear");

  ds_vector_destroy(v, NULL);
}

TEST_FUNC(test_vector_bulk_growth_and_integrity) {
  ds_vector_t *v = ds_vector_create(1);
  ASSERT_NOT_NULL(v, "create(1)");

  const int N = 5000;
  for (int i = 0; i < N; i++) {
    ds_status_t st = ds_vector_push_back(v, mk_int(i));
    ASSERT_EQ(st, DS_OK, "push_back in bulk should succeed");
  }

  ASSERT_EQ(ds_vector_size(v), (size_t)N, "bulk size == N");
  ASSERT(ds_vector_capacity(v) >= (size_t)N, "capacity >= N after growth");

  ASSERT_EQ(int_val(ds_vector_get(v, 0)), 0, "bulk get(0) == 0");
  ASSERT_EQ(int_val(ds_vector_get(v, N/2)), N/2, "bulk get(mid) correct");
  ASSERT_EQ(int_val(ds_vector_get(v, N-1)), N-1, "bulk get(last) correct");

  for (int i = 0; i < 100; i++) {
    int *p = (int *)ds_vector_pop_back(v);
    ASSERT_NOT_NULL(p, "bulk pop_back non-NULL");
    free(p);
  }
  ASSERT_EQ(ds_vector_size(v), (size_t)(N - 100), "size after popping 100");

  ds_vector_destroy(v, counted_free);
}

/* ===================== main ===================== */

int main() {
  test_case_t tests[] = {
    {"vector_create_destroy_basic", test_vector_create_destroy_basic},
    {"vector_null_safety_queries", test_vector_null_safety_queries},
    {"vector_error_codes_null_and_arg", test_vector_error_codes_null_and_arg},
    {"vector_push_get_pop", test_vector_push_get_pop},
    {"vector_insert_middle_and_order", test_vector_insert_middle_and_order},
    {"vector_reserve_behavior", test_vector_reserve_behavior},
    {"vector_set_replaces_and_frees_old", test_vector_set_replaces_and_frees_old},
    {"vector_remove_and_shift", test_vector_remove_and_shift},
    {"vector_clear_frees_all_and_keeps_capacity", test_vector_clear_frees_all_and_keeps_capacity},
    {"vector_bulk_growth_and_integrity", test_vector_bulk_growth_and_integrity},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
