#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ds_list.h"
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

/* 断言 list 内容等于给定数组（从头遍历） */
static void assert_list_equals(ds_list_t *list, const int *arr, size_t n, const char *msg_prefix) {
  ASSERT_EQ(ds_list_size(list), n, "list size matches expected");

  ds_list_iter_t it = ds_list_iter_begin(list);
  for (size_t i = 0; i < n; i++) {
    ASSERT_NOT_NULL(it, "iterator not NULL while expecting elements");
    int v = int_val(ds_list_iter_get(it));

    char buf[128];
    snprintf(buf, sizeof(buf), "%s forward[%zu] == %d", msg_prefix, i, arr[i]);
    ASSERT_EQ(v, arr[i], buf);

    it = ds_list_iter_next(it);
  }
  ASSERT_NULL(it, "iterator reaches end after n steps");
}

/* ===================== Tests ===================== */

TEST_FUNC(test_list_create_destroy_basic) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create() returns non-NULL");
  ASSERT_EQ(ds_list_size(l), 0u, "new list size == 0");
  ASSERT(ds_list_is_empty(l) == true, "new list is empty");

  ds_list_destroy(l, NULL);
  ASSERT(1, "destroy(list,NULL) does not crash");

  ds_list_destroy(NULL, NULL);
  ASSERT(1, "destroy(NULL) does not crash");
}

TEST_FUNC(test_list_null_safety_queries) {
  ASSERT_EQ(ds_list_size(NULL), 0u, "size(NULL) == 0");
  ASSERT(ds_list_is_empty(NULL) == true, "is_empty(NULL) == true");

  ASSERT_NULL(ds_list_iter_begin(NULL), "iter_begin(NULL) == NULL");
  ASSERT_NULL(ds_list_iter_tail(NULL), "iter_tail(NULL) == NULL");
  ASSERT(ds_list_iter_end(NULL) == NULL, "iter_end(NULL) == NULL");

  ASSERT_NULL(ds_list_iter_next(NULL), "iter_next(NULL) == NULL");
  ASSERT_NULL(ds_list_iter_prev(NULL), "iter_prev(NULL) == NULL");
  ASSERT_NULL(ds_list_iter_get(NULL), "iter_get(NULL) == NULL");
  ASSERT(ds_list_iter_equal(NULL, NULL) == false, "iter_equal(NULL,NULL) == false (current behavior)");

  ds_list_clear(NULL, NULL);
  ASSERT(1, "clear(NULL) does not crash");
}

TEST_FUNC(test_list_push_front_back_order) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  ASSERT_EQ(ds_list_push_back(l, mk_int(1)), DS_OK, "push_back 1");
  ASSERT_EQ(ds_list_push_back(l, mk_int(2)), DS_OK, "push_back 2");
  ASSERT_EQ(ds_list_push_front(l, mk_int(0)), DS_OK, "push_front 0");

  int expected[] = {0,1,2};
  assert_list_equals(l, expected, 3, "push_front_back");

  ds_list_destroy(l, counted_free);
}

TEST_FUNC(test_list_insert_cases) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  /* Case: insert into empty with it == NULL acts like push_back */
  ASSERT_EQ(ds_list_insert(l, NULL, mk_int(10)), DS_OK, "insert(empty, NULL, 10) OK");
  int expected1[] = {10};
  assert_list_equals(l, expected1, 1, "insert empty");

  /* Case: insert at head (it == head) */
  ds_list_iter_t beg = ds_list_iter_begin(l);
  ASSERT_NOT_NULL(beg, "begin non-NULL");
  ASSERT_EQ(ds_list_insert(l, beg, mk_int(5)), DS_OK, "insert at head before 10");
  int expected2[] = {5,10};
  assert_list_equals(l, expected2, 2, "insert head");

  /* Case: insert at end (it == NULL) */
  ASSERT_EQ(ds_list_insert(l, NULL, mk_int(20)), DS_OK, "insert at end 20");
  int expected3[] = {5,10,20};
  assert_list_equals(l, expected3, 3, "insert end");

  /* Case: insert before middle iterator (before 20) */
  ds_list_iter_t it = ds_list_iter_begin(l);
  it = ds_list_iter_next(it); /* points to 10 */
  it = ds_list_iter_next(it); /* points to 20 */
  ASSERT_EQ(int_val(ds_list_iter_get(it)), 20, "it points to 20");
  ASSERT_EQ(ds_list_insert(l, it, mk_int(15)), DS_OK, "insert 15 before 20");
  int expected4[] = {5,10,15,20};
  assert_list_equals(l, expected4, 4, "insert middle");

  ds_list_destroy(l, counted_free);
}

TEST_FUNC(test_list_iter_forward_backward) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  for (int i = 1; i <= 5; i++) {
    ASSERT_EQ(ds_list_push_back(l, mk_int(i)), DS_OK, "push_back i");
  }

  /* forward */
  int exp_fwd[] = {1,2,3,4,5};
  assert_list_equals(l, exp_fwd, 5, "iter forward");

  /* backward using tail/prev */
  ds_list_iter_t it = ds_list_iter_tail(l);
  ASSERT_NOT_NULL(it, "tail non-NULL");
  for (int i = 5; i >= 1; i--) {
    ASSERT_NOT_NULL(it, "backward it non-NULL");
    ASSERT_EQ(int_val(ds_list_iter_get(it)), i, "backward value matches");
    it = ds_list_iter_prev(it);
  }
  ASSERT_NULL(it, "prev from head leads to NULL");

  ds_list_destroy(l, counted_free);
}

TEST_FUNC(test_list_remove_head_tail_middle) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  for (int i = 1; i <= 5; i++) ds_list_push_back(l, mk_int(i));
  int exp0[] = {1,2,3,4,5};
  assert_list_equals(l, exp0, 5, "before remove");

  /* remove head */
  ds_list_iter_t it = ds_list_iter_begin(l);
  ASSERT_EQ(int_val(ds_list_iter_get(it)), 1, "head == 1");
  int free_before = g_free_count;
  ds_list_iter_t next = ds_list_remove(l, it, counted_free);
  ASSERT_EQ(g_free_count, free_before + 1, "remove head freed one element");
  ASSERT_NOT_NULL(next, "remove head returns next iterator");
  ASSERT_EQ(int_val(ds_list_iter_get(next)), 2, "next is 2");
  int exp1[] = {2,3,4,5};
  assert_list_equals(l, exp1, 4, "after remove head");

  /* remove middle (value 4) */
  it = ds_list_iter_begin(l);          /* 2 */
  it = ds_list_iter_next(it);          /* 3 */
  it = ds_list_iter_next(it);          /* 4 */
  ASSERT_EQ(int_val(ds_list_iter_get(it)), 4, "middle it == 4");
  free_before = g_free_count;
  next = ds_list_remove(l, it, counted_free);
  ASSERT_EQ(g_free_count, free_before + 1, "remove middle freed one element");
  ASSERT_NOT_NULL(next, "remove middle returns next");
  ASSERT_EQ(int_val(ds_list_iter_get(next)), 5, "next is 5");
  int exp2[] = {2,3,5};
  assert_list_equals(l, exp2, 3, "after remove middle");

  /* remove tail (value 5) */
  it = ds_list_iter_tail(l);
  ASSERT_EQ(int_val(ds_list_iter_get(it)), 5, "tail == 5");
  free_before = g_free_count;
  next = ds_list_remove(l, it, counted_free);
  ASSERT_EQ(g_free_count, free_before + 1, "remove tail freed one element");
  ASSERT_NULL(next, "remove tail returns NULL");
  int exp3[] = {2,3};
  assert_list_equals(l, exp3, 2, "after remove tail");

  /* remove with it == NULL is no-op, returns NULL */
  free_before = g_free_count;
  next = ds_list_remove(l, NULL, counted_free);
  ASSERT_NULL(next, "remove(NULL iterator) returns NULL");
  ASSERT_EQ(g_free_count, free_before, "remove(NULL iterator) does not free");

  ds_list_destroy(l, counted_free);
}

TEST_FUNC(test_list_remove_singleton) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  ds_list_push_back(l, mk_int(42));
  ASSERT_EQ(ds_list_size(l), 1u, "size == 1");
  ds_list_iter_t it = ds_list_iter_begin(l);
  ASSERT_NOT_NULL(it, "begin non-NULL");
  int free_before = g_free_count;
  ds_list_iter_t next = ds_list_remove(l, it, counted_free);
  ASSERT_EQ(g_free_count, free_before + 1, "remove singleton frees one");
  ASSERT_NULL(next, "remove singleton returns NULL");
  ASSERT_EQ(ds_list_size(l), 0u, "size == 0 after remove");
  ASSERT(ds_list_is_empty(l) == true, "empty after remove");
  ASSERT_NULL(ds_list_iter_begin(l), "begin NULL after remove");

  ds_list_destroy(l, counted_free);
}

TEST_FUNC(test_list_clear_and_destroy_free_counts) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  for (int i = 0; i < 10; i++) ds_list_push_back(l, mk_int(i));
  ASSERT_EQ(ds_list_size(l), 10u, "size == 10 before clear");

  int free_before = g_free_count;
  ds_list_clear(l, counted_free);
  ASSERT_EQ(g_free_count, free_before + 10, "clear freed 10 elements");
  ASSERT_EQ(ds_list_size(l), 0u, "size == 0 after clear");
  ASSERT(ds_list_is_empty(l) == true, "empty after clear");
  ASSERT_NULL(ds_list_iter_begin(l), "begin NULL after clear");
  ASSERT_NULL(ds_list_iter_tail(l), "tail NULL after clear");

  ds_list_destroy(l, counted_free);
}

TEST_FUNC(test_list_set_replaces_and_frees_old) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  ds_list_push_back(l, mk_int(7));
  ds_list_iter_t it = ds_list_iter_begin(l);
  ASSERT_NOT_NULL(it, "begin non-NULL");

  int free_before = g_free_count;
  ds_status_t st = ds_list_set(l, it, mk_int(8), counted_free);
  ASSERT_EQ(st, DS_OK, "set returns DS_OK");
  ASSERT_EQ(g_free_count, free_before + 1, "set freed old element once");
  ASSERT_EQ(int_val(ds_list_iter_get(it)), 8, "value updated to 8");

  /* 参数检查：失败路径不接管 element，所以必须手动 free */

  /* list == NULL */
  int *tmp = mk_int(1);
  ASSERT_NOT_NULL(tmp, "mk_int(1)");
  ASSERT_EQ(ds_list_set(NULL, it, tmp, counted_free), DS_ERR_NULL, "set(NULL,...) => DS_ERR_NULL");
  free(tmp);

  /* it == NULL */
  tmp = mk_int(9);
  ASSERT_NOT_NULL(tmp, "mk_int(9)");
  ASSERT_EQ(ds_list_set(l, NULL, tmp, counted_free), DS_ERR_ARG, "set(it==NULL) => DS_ERR_ARG");
  free(tmp);

  /* element == NULL */
  ASSERT_EQ(ds_list_set(l, it, NULL, counted_free), DS_ERR_ARG, "set(element==NULL) => DS_ERR_ARG");

  /* 先把旧指针拿出来保存 */
  int *old = (int *)ds_list_iter_get(it);
  tmp = mk_int(11);
  ASSERT_NOT_NULL(tmp, "mk_int(11)");
  ASSERT_EQ(ds_list_set(l, it, tmp, NULL), DS_OK, "set with NULL old_element_free should be OK");

  /* 因为 set 不会 free old，所以测试自己 free old，避免 valgrind 报 leak */
  free(old);

  ds_list_destroy(l, counted_free);
}

/* pop_front / pop_back 的测试：这里会直接暴露你当前实现的 bug
   - pop_front 没有 size--
   - pop_back/pop_front 在 size==0 时会解引用 NULL
 */
TEST_FUNC(test_list_pop_front_back_behavior) {
  ds_list_t *l = ds_list_create();
  ASSERT_NOT_NULL(l, "create list");

  /* 空 pop 应该安全返回 NULL（你当前实现会崩：tmp = list->tail/head 为 NULL 仍取 data）
     所以这里先不调用空 pop，避免直接 crash。你修完后可以解除注释。 */

  /*
  ASSERT_NULL(ds_list_pop_front(l), "pop_front on empty returns NULL");
  ASSERT_NULL(ds_list_pop_back(l), "pop_back on empty returns NULL");
  */

  ds_list_push_back(l, mk_int(1));
  ds_list_push_back(l, mk_int(2));
  ds_list_push_back(l, mk_int(3));

  /* pop_front should return 1 */
  int *p = (int *)ds_list_pop_front(l);
  ASSERT_NOT_NULL(p, "pop_front returns non-NULL");
  ASSERT_EQ(*p, 1, "pop_front == 1");
  free(p);

  /* 如果你修了 pop_front 的 size--，这里应该是 2 */
  ASSERT_EQ(ds_list_size(l), 2u, "size after pop_front should be 2 (will fail until you fix pop_front)");

  /* pop_back should return 3 */
  p = (int *)ds_list_pop_back(l);
  ASSERT_NOT_NULL(p, "pop_back returns non-NULL");
  ASSERT_EQ(*p, 3, "pop_back == 3");
  free(p);

  ASSERT_EQ(ds_list_size(l), 1u, "size after pop_back should be 1");

  /* remaining element is 2 */
  ds_list_iter_t it = ds_list_iter_begin(l);
  ASSERT_NOT_NULL(it, "begin non-NULL");
  ASSERT_EQ(int_val(ds_list_iter_get(it)), 2, "remaining == 2");

  ds_list_destroy(l, counted_free);
}

int main() {
  test_case_t tests[] = {
    {"list_create_destroy_basic", test_list_create_destroy_basic},
    {"list_null_safety_queries", test_list_null_safety_queries},
    {"list_push_front_back_order", test_list_push_front_back_order},
    {"list_insert_cases", test_list_insert_cases},
    {"list_iter_forward_backward", test_list_iter_forward_backward},
    {"list_remove_head_tail_middle", test_list_remove_head_tail_middle},
    {"list_remove_singleton", test_list_remove_singleton},
    {"list_clear_and_destroy_free_counts", test_list_clear_and_destroy_free_counts},
    {"list_set_replaces_and_frees_old", test_list_set_replaces_and_frees_old},
    {"list_pop_front_back_behavior", test_list_pop_front_back_behavior},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
