#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ds_deque.h"
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

/* 一个简单的 reference deque（用动态数组模拟） */
typedef struct {
  int *a;
  size_t cap;
  size_t n;
  size_t head; /* index of first valid */
} ref_deque_t;

static void ref_init(ref_deque_t *r, size_t cap) {
  r->cap = cap ? cap : 16;
  r->a = (int *)malloc(sizeof(int) * r->cap);
  r->n = 0;
  r->head = 0;
}

static void ref_free(ref_deque_t *r) {
  free(r->a);
  r->a = NULL;
  r->cap = r->n = r->head = 0;
}

static void ref_grow(ref_deque_t *r) {
  size_t newcap = r->cap * 2;
  int *na = (int *)malloc(sizeof(int) * newcap);
  for (size_t i = 0; i < r->n; i++) {
    na[i] = r->a[(r->head + i) % r->cap];
  }
  free(r->a);
  r->a = na;
  r->cap = newcap;
  r->head = 0;
}

static void ref_push_back(ref_deque_t *r, int v) {
  if (r->n == r->cap) ref_grow(r);
  size_t idx = (r->head + r->n) % r->cap;
  r->a[idx] = v;
  r->n++;
}

static void ref_push_front(ref_deque_t *r, int v) {
  if (r->n == r->cap) ref_grow(r);
  r->head = (r->head + r->cap - 1) % r->cap;
  r->a[r->head] = v;
  r->n++;
}

static int ref_pop_back(ref_deque_t *r, int *ok) {
  if (r->n == 0) { *ok = 0; return 0; }
  size_t idx = (r->head + r->n - 1) % r->cap;
  int v = r->a[idx];
  r->n--;
  *ok = 1;
  return v;
}

static int ref_pop_front(ref_deque_t *r, int *ok) {
  if (r->n == 0) { *ok = 0; return 0; }
  int v = r->a[r->head];
  r->head = (r->head + 1) % r->cap;
  r->n--;
  *ok = 1;
  return v;
}

static int ref_front(ref_deque_t *r, int *ok) {
  if (r->n == 0) { *ok = 0; return 0; }
  *ok = 1;
  return r->a[r->head];
}

static int ref_back(ref_deque_t *r, int *ok) {
  if (r->n == 0) { *ok = 0; return 0; }
  size_t idx = (r->head + r->n - 1) % r->cap;
  *ok = 1;
  return r->a[idx];
}

/* ===================== Tests ===================== */

TEST_FUNC(test_deque_create_destroy_basic) {
  ds_deque_t *d = ds_deque_create(0);
  ASSERT_NOT_NULL(d, "create(0) non-NULL");
  ASSERT_EQ(ds_deque_size(d), 0u, "new deque size == 0");
  ASSERT(ds_deque_is_empty(d) == true, "new deque empty");
  ASSERT(ds_deque_capacity(d) >= 1u, "capacity >= 1");
  ds_deque_destroy(d, NULL);
  ASSERT(1, "destroy ok");
}

TEST_FUNC(test_deque_null_safety) {
  ASSERT_EQ(ds_deque_size(NULL), 0u, "size(NULL)==0");
  ASSERT_EQ(ds_deque_capacity(NULL), 0u, "capacity(NULL)==0");
  ASSERT(ds_deque_is_empty(NULL) == true, "is_empty(NULL)==true");

  int *p = mk_int(1);
  ASSERT_NOT_NULL(p, "mk_int");
  ASSERT_EQ(ds_deque_push_back(NULL, p), DS_ERR_NULL, "push_back(NULL,elem)==DS_ERR_NULL");
  ASSERT_EQ(ds_deque_push_front(NULL, p), DS_ERR_NULL, "push_front(NULL,elem)==DS_ERR_NULL");
  free(p);

  ASSERT_NULL(ds_deque_pop_back(NULL), "pop_back(NULL)==NULL");
  ASSERT_NULL(ds_deque_pop_front(NULL), "pop_front(NULL)==NULL");
  ASSERT_NULL(ds_deque_front(NULL), "front(NULL)==NULL");
  ASSERT_NULL(ds_deque_back(NULL), "back(NULL)==NULL");

  ds_deque_clear(NULL, NULL);
  ASSERT(1, "clear(NULL) ok");
  ds_deque_destroy(NULL, NULL);
  ASSERT(1, "destroy(NULL) ok");
}

TEST_FUNC(test_deque_arg_checks) {
  ds_deque_t *d = ds_deque_create(2);
  ASSERT_NOT_NULL(d, "create(2)");

  ASSERT_EQ(ds_deque_push_back(d, NULL), DS_ERR_ARG, "push_back(NULL elem)==DS_ERR_ARG");
  ASSERT_EQ(ds_deque_push_front(d, NULL), DS_ERR_ARG, "push_front(NULL elem)==DS_ERR_ARG");

  ds_deque_destroy(d, NULL);
}

TEST_FUNC(test_deque_push_pop_front_back_simple) {
  ds_deque_t *d = ds_deque_create(4);
  ASSERT_NOT_NULL(d, "create(4)");

  ASSERT_NULL(ds_deque_front(d), "front(empty)==NULL");
  ASSERT_NULL(ds_deque_back(d), "back(empty)==NULL");

  ASSERT_EQ(ds_deque_push_back(d, mk_int(1)), DS_OK, "push_back 1");
  ASSERT_EQ(ds_deque_push_back(d, mk_int(2)), DS_OK, "push_back 2");
  ASSERT_EQ(ds_deque_push_front(d, mk_int(0)), DS_OK, "push_front 0");

  ASSERT_EQ(ds_deque_size(d), 3u, "size==3");
  ASSERT_EQ(int_val(ds_deque_front(d)), 0, "front==0");
  ASSERT_EQ(int_val(ds_deque_back(d)), 2, "back==2");

  int *p = (int *)ds_deque_pop_front(d);
  ASSERT_NOT_NULL(p, "pop_front non-NULL");
  ASSERT_EQ(*p, 0, "pop_front==0");
  free(p);

  p = (int *)ds_deque_pop_back(d);
  ASSERT_NOT_NULL(p, "pop_back non-NULL");
  ASSERT_EQ(*p, 2, "pop_back==2");
  free(p);

  p = (int *)ds_deque_pop_back(d);
  ASSERT_NOT_NULL(p, "pop_back last non-NULL");
  ASSERT_EQ(*p, 1, "pop_back==1");
  free(p);

  ASSERT_EQ(ds_deque_size(d), 0u, "size==0 after pops");
  ASSERT(ds_deque_is_empty(d) == true, "empty after pops");
  ASSERT_NULL(ds_deque_pop_front(d), "pop_front on empty == NULL");
  ASSERT_NULL(ds_deque_pop_back(d), "pop_back on empty == NULL");

  ds_deque_destroy(d, counted_free);
}

TEST_FUNC(test_deque_wraparound_no_grow) {
  ds_deque_t *d = ds_deque_create(4);
  ASSERT_NOT_NULL(d, "create(4)");

  /* Fill: [1,2,3,4] */
  ds_deque_push_back(d, mk_int(1));
  ds_deque_push_back(d, mk_int(2));
  ds_deque_push_back(d, mk_int(3));
  ds_deque_push_back(d, mk_int(4));
  ASSERT_EQ(ds_deque_size(d), 4u, "size==4 full");

  /* pop 2 from front -> head moves forward */
  int *p = (int *)ds_deque_pop_front(d); free(p);
  p = (int *)ds_deque_pop_front(d); free(p);
  ASSERT_EQ(ds_deque_size(d), 2u, "size==2 after pop_front twice");
  ASSERT_EQ(int_val(ds_deque_front(d)), 3, "front==3");
  ASSERT_EQ(int_val(ds_deque_back(d)), 4, "back==4");

  /* push_back 5,6 should wrap tail */
  ds_deque_push_back(d, mk_int(5));
  ds_deque_push_back(d, mk_int(6));
  ASSERT_EQ(ds_deque_size(d), 4u, "size==4 after wrap pushes");
  ASSERT_EQ(int_val(ds_deque_front(d)), 3, "front==3 after wrap");
  ASSERT_EQ(int_val(ds_deque_back(d)), 6, "back==6 after wrap");

  /* Now pop all and check order: 3,4,5,6 */
  int exp[] = {3,4,5,6};
  for (int i = 0; i < 4; i++) {
    p = (int *)ds_deque_pop_front(d);
    ASSERT_NOT_NULL(p, "pop_front non-NULL");
    ASSERT_EQ(*p, exp[i], "wrap pop order matches");
    free(p);
  }
  ASSERT_EQ(ds_deque_size(d), 0u, "empty after popping all");

  ds_deque_destroy(d, counted_free);
}

TEST_FUNC(test_deque_grow_preserves_order) {
  ds_deque_t *d = ds_deque_create(2);
  ASSERT_NOT_NULL(d, "create(2)");

  /* Force head!=0 then grow */
  ds_deque_push_back(d, mk_int(1));
  ds_deque_push_back(d, mk_int(2));
  int *p = (int *)ds_deque_pop_front(d); free(p); /* head moved */
  ds_deque_push_back(d, mk_int(3));              /* wrap likely */
  ASSERT_EQ(ds_deque_size(d), 2u, "size==2 before grow");

  size_t cap_before = ds_deque_capacity(d);
  ASSERT_EQ(ds_deque_push_back(d, mk_int(4)), DS_OK, "push_back triggers grow or fits");
  ASSERT(ds_deque_capacity(d) >= cap_before, "capacity non-decreasing");

  /* Check order should be [2?] Actually after operations:
     start: [1,2]
     pop_front -> removes 1, remaining [2]
     push_back 3 -> [2,3]
     push_back 4 -> [2,3,4]
   */
  ASSERT_EQ(int_val(ds_deque_front(d)), 2, "front==2 after grow");
  ASSERT_EQ(int_val(ds_deque_back(d)), 4, "back==4 after grow");

  int exp[] = {2,3,4};
  for (int i = 0; i < 3; i++) {
    p = (int *)ds_deque_pop_front(d);
    ASSERT_NOT_NULL(p, "pop_front non-NULL");
    ASSERT_EQ(*p, exp[i], "grow pop order matches");
    free(p);
  }

  ds_deque_destroy(d, counted_free);
}

TEST_FUNC(test_deque_clear_and_destroy_free) {
  ds_deque_t *d = ds_deque_create(0);
  ASSERT_NOT_NULL(d, "create default");

  for (int i = 0; i < 10; i++) ds_deque_push_back(d, mk_int(i));
  ASSERT_EQ(ds_deque_size(d), 10u, "size==10");

  int free_before = g_free_count;
  ds_deque_clear(d, counted_free);
  ASSERT_EQ(g_free_count, free_before + 10, "clear frees 10");
  ASSERT_EQ(ds_deque_size(d), 0u, "size==0 after clear");
  ASSERT(ds_deque_is_empty(d) == true, "empty after clear");
  ASSERT_NULL(ds_deque_front(d), "front NULL after clear");
  ASSERT_NULL(ds_deque_back(d), "back NULL after clear");

  ds_deque_destroy(d, counted_free);
}

TEST_FUNC(test_deque_random_ops_against_reference) {
  ds_deque_t *d = ds_deque_create(4);
  ASSERT_NOT_NULL(d, "create(4)");

  ref_deque_t r;
  ref_init(&r, 4);

  /* 固定 seed，保证可复现 */
  unsigned seed = 1234567u;
  const int OPS = 20000;

  for (int step = 0; step < OPS; step++) {
    seed = seed * 1103515245u + 12345u;
    int op = (seed >> 16) % 6; /* 0..5 */

    if (op == 0) { /* push_back */
      int val = (int)(seed & 0x7fffffff);
      ds_status_t st = ds_deque_push_back(d, mk_int(val));
      ASSERT_EQ(st, DS_OK, "random push_back DS_OK");
      ref_push_back(&r, val);
    } else if (op == 1) { /* push_front */
      int val = (int)(seed & 0x7fffffff);
      ds_status_t st = ds_deque_push_front(d, mk_int(val));
      ASSERT_EQ(st, DS_OK, "random push_front DS_OK");
      ref_push_front(&r, val);
    } else if (op == 2) { /* pop_back */
      int ok;
      int rv = ref_pop_back(&r, &ok);
      int *pv = (int *)ds_deque_pop_back(d);
      if (!ok) {
        ASSERT_NULL(pv, "random pop_back: empty => NULL");
      } else {
        ASSERT_NOT_NULL(pv, "random pop_back non-NULL");
        ASSERT_EQ(*pv, rv, "random pop_back matches reference");
        free(pv);
      }
    } else if (op == 3) { /* pop_front */
      int ok;
      int rv = ref_pop_front(&r, &ok);
      int *pv = (int *)ds_deque_pop_front(d);
      if (!ok) {
        ASSERT_NULL(pv, "random pop_front: empty => NULL");
      } else {
        ASSERT_NOT_NULL(pv, "random pop_front non-NULL");
        ASSERT_EQ(*pv, rv, "random pop_front matches reference");
        free(pv);
      }
    } else if (op == 4) { /* front */
      int ok;
      int rv = ref_front(&r, &ok);
      int *pv = (int *)ds_deque_front(d);
      if (!ok) {
        ASSERT_NULL(pv, "random front: empty => NULL");
      } else {
        ASSERT_NOT_NULL(pv, "random front non-NULL");
        ASSERT_EQ(*pv, rv, "random front matches reference");
      }
    } else { /* back */
      int ok;
      int rv = ref_back(&r, &ok);
      int *pv = (int *)ds_deque_back(d);
      if (!ok) {
        ASSERT_NULL(pv, "random back: empty => NULL");
      } else {
        ASSERT_NOT_NULL(pv, "random back non-NULL");
        ASSERT_EQ(*pv, rv, "random back matches reference");
      }
    }

    ASSERT_EQ(ds_deque_size(d), r.n, "size matches reference");
  }

  /* cleanup: free remaining elements inside deque */
  ds_deque_destroy(d, counted_free);
  ref_free(&r);
}

/* ===================== main ===================== */

int main() {
  test_case_t tests[] = {
    {"deque_create_destroy_basic", test_deque_create_destroy_basic},
    {"deque_null_safety", test_deque_null_safety},
    {"deque_arg_checks", test_deque_arg_checks},
    {"deque_push_pop_front_back_simple", test_deque_push_pop_front_back_simple},
    {"deque_wraparound_no_grow", test_deque_wraparound_no_grow},
    {"deque_grow_preserves_order", test_deque_grow_preserves_order},
    {"deque_clear_and_destroy_free", test_deque_clear_and_destroy_free},
    {"deque_random_ops_against_reference", test_deque_random_ops_against_reference},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
