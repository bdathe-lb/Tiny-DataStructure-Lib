#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ds_bst.h"
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

static int int_val(const void *p) {
  return p ? *(const int *)p : 0;
}

/* compare for ints stored as int* */
static int int_compare(const void *a, const void *b) {
  int x = *(const int *)a;
  int y = *(const int *)b;
  if (x < y) return -1;
  if (x > y) return 1;
  return 0;
}

static int g_free_count = 0;
static void counted_free(void *p) {
  if (p) g_free_count++;
  free(p);
}

/* collect inorder output */
static int g_out[4096];
static size_t g_out_n = 0;

static void out_reset(void) {
  g_out_n = 0;
  memset(g_out, 0, sizeof(g_out));
}

static void visit_collect(void *data) {
  if (g_out_n < sizeof(g_out)/sizeof(g_out[0])) {
    g_out[g_out_n++] = *(int *)data;
  }
}

static void assert_sorted_strict_increasing(const int *arr, size_t n, const char *msg) {
  if (n == 0) { ASSERT(1, msg); return; }
  for (size_t i = 1; i < n; i++) {
    if (!(arr[i-1] < arr[i])) {
      ASSERT(0, msg);
      return;
    }
  }
  ASSERT(1, msg);
}

/* ===================== Tests ===================== */

TEST_FUNC(test_bst_create_basic) {
  ds_bst_t *b = ds_bst_create(NULL);
  ASSERT_NULL(b, "create(NULL compare) returns NULL");

  b = ds_bst_create(int_compare);
  ASSERT_NOT_NULL(b, "create(compare) non-NULL");
  ASSERT_EQ(ds_bst_size(b), 0u, "new bst size==0");
  ASSERT_NULL(ds_bst_min(b), "min(empty)==NULL");
  ASSERT_NULL(ds_bst_max(b), "max(empty)==NULL");

  int key = 1;
  ASSERT_NULL(ds_bst_search(b, &key), "search(empty)==NULL");

  ds_bst_destroy(b, NULL);
}

TEST_FUNC(test_bst_insert_search_min_max_inorder) {
  ds_bst_t *b = ds_bst_create(int_compare);
  ASSERT_NOT_NULL(b, "create");

  int vals[] = {5, 3, 7, 2, 4, 6, 8};
  for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
    ASSERT_EQ(ds_bst_insert(b, mk_int(vals[i])), DS_OK, "insert OK");
  }
  ASSERT_EQ(ds_bst_size(b), 7u, "size==7");

  int k2 = 2, k8 = 8, k4 = 4, k10 = 10;
  ASSERT_EQ(int_val(ds_bst_search(b, &k2)), 2, "search 2 found");
  ASSERT_EQ(int_val(ds_bst_search(b, &k8)), 8, "search 8 found");
  ASSERT_EQ(int_val(ds_bst_search(b, &k4)), 4, "search 4 found");
  ASSERT_NULL(ds_bst_search(b, &k10), "search 10 not found");

  ASSERT_EQ(int_val(ds_bst_min(b)), 2, "min==2");
  ASSERT_EQ(int_val(ds_bst_max(b)), 8, "max==8");

  out_reset();
  ds_bst_traverse_inorder(b, visit_collect);
  ASSERT_EQ(g_out_n, 7u, "inorder visits 7");
  assert_sorted_strict_increasing(g_out, g_out_n, "inorder is strictly increasing");

  ds_bst_destroy(b, counted_free);
}

TEST_FUNC(test_bst_insert_duplicate_key) {
  ds_bst_t *b = ds_bst_create(int_compare);
  ASSERT_NOT_NULL(b, "create");

  ASSERT_EQ(ds_bst_insert(b, mk_int(1)), DS_OK, "insert 1 OK");

  int *dup = mk_int(1);
  ASSERT_NOT_NULL(dup, "mk_int dup");
  ASSERT_EQ(ds_bst_insert(b, dup), DS_ERR_EXIST, "insert duplicate => DS_ERR_EXIST");
  /* 插入失败不接管所有权，调用者必须释放 dup */
  free(dup);

  ASSERT_EQ(ds_bst_size(b), 1u, "size still 1");

  ds_bst_destroy(b, counted_free);
}

TEST_FUNC(test_bst_remove_leaf_one_child_two_children) {
  ds_bst_t *b = ds_bst_create(int_compare);
  ASSERT_NOT_NULL(b, "create");

  /* 构造一个常见结构：
        5
      /   \
     3     7
    / \   / \
   2  4  6  8
   */
  int vals[] = {5,3,7,2,4,6,8};
  for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
    ASSERT_EQ(ds_bst_insert(b, mk_int(vals[i])), DS_OK, "insert");
  }
  ASSERT_EQ(ds_bst_size(b), 7u, "size==7");

  /* 1) remove leaf: 2 */
  int k2 = 2;
  int free_before = g_free_count;
  ASSERT_EQ(ds_bst_remove(b, &k2, counted_free), DS_OK, "remove leaf 2 OK");
  ASSERT_EQ(g_free_count, free_before + 1, "remove leaf frees one data");
  ASSERT_EQ(ds_bst_size(b), 6u, "size==6 after remove 2");
  ASSERT_NULL(ds_bst_search(b, &k2), "2 not found after remove");

  /* 2) make one-child case: remove 8 (leaf), then remove 7 (has only left child 6) */
  int k8 = 8;
  free_before = g_free_count;
  ASSERT_EQ(ds_bst_remove(b, &k8, counted_free), DS_OK, "remove leaf 8 OK");
  ASSERT_EQ(g_free_count, free_before + 1, "remove 8 frees one");
  ASSERT_EQ(ds_bst_size(b), 5u, "size==5");

  int k7 = 7;
  free_before = g_free_count;
  ASSERT_EQ(ds_bst_remove(b, &k7, counted_free), DS_OK, "remove node 7 (one child) OK");
  ASSERT_EQ(g_free_count, free_before + 1, "remove 7 frees one");
  ASSERT_EQ(ds_bst_size(b), 4u, "size==4");
  ASSERT_NULL(ds_bst_search(b, &k7), "7 not found");

  /* 3) two-children: remove 5 (root, has two children) */
  int k5 = 5;
  free_before = g_free_count;
  ASSERT_EQ(ds_bst_remove(b, &k5, counted_free), DS_OK, "remove 5 (two children) OK");
  ASSERT_EQ(g_free_count, free_before + 1, "remove 5 frees one");
  ASSERT_EQ(ds_bst_size(b), 3u, "size==3");
  ASSERT_NULL(ds_bst_search(b, &k5), "5 not found");

  /* inorder still strictly increasing */
  out_reset();
  ds_bst_traverse_inorder(b, visit_collect);
  assert_sorted_strict_increasing(g_out, g_out_n, "inorder sorted after removals");

  ds_bst_destroy(b, counted_free);
}

TEST_FUNC(test_bst_remove_errors) {
  ds_bst_t *b = ds_bst_create(int_compare);
  ASSERT_NOT_NULL(b, "create");

  int k = 1;
  ASSERT_EQ(ds_bst_remove(NULL, &k, NULL), DS_ERR_NULL, "remove(NULL bst) => DS_ERR_NULL");
  ASSERT_EQ(ds_bst_remove(b, &k, NULL), DS_ERR_NOT_FOUND, "remove(empty) => DS_ERR_NOT_FOUND");
  ASSERT_EQ(ds_bst_remove(b, NULL, NULL), DS_ERR_NOT_FOUND /* root empty checked first */, "remove(empty, NULL key) => DS_ERR_NOT_FOUND (current behavior)");

  /* insert one then test key NULL */
  ASSERT_EQ(ds_bst_insert(b, mk_int(1)), DS_OK, "insert 1");
  ASSERT_EQ(ds_bst_remove(b, NULL, NULL), DS_ERR_ARG, "remove(NULL key) => DS_ERR_ARG when non-empty");

  int k2 = 2;
  ASSERT_EQ(ds_bst_remove(b, &k2, NULL), DS_ERR_NOT_FOUND, "remove(nonexistent) => DS_ERR_NOT_FOUND");

  ds_bst_destroy(b, counted_free);
}

TEST_FUNC(test_bst_random_insert_delete_inorder_sorted) {
  ds_bst_t *b = ds_bst_create(int_compare);
  ASSERT_NOT_NULL(b, "create");

  /* 用一个小范围 key，方便用存在性数组做 reference */
  enum { MAXK = 2000 };
  unsigned char present[MAXK];
  memset(present, 0, sizeof(present));

  uint32_t seed = 0x12345678u;
  const int OPS = 20000;

  for (int step = 0; step < OPS; step++) {
    seed = seed * 1664525u + 1013904223u;
    int key = (int)(seed % MAXK);

    int op = (seed >> 16) & 1; /* 0 insert, 1 remove */
    if (op == 0) {
      if (!present[key]) {
        ds_status_t st = ds_bst_insert(b, mk_int(key));
        ASSERT_EQ(st, DS_OK, "random insert DS_OK");
        present[key] = 1;
      } else {
        int *dup = mk_int(key);
        ASSERT_NOT_NULL(dup, "dup alloc");
        ds_status_t st = ds_bst_insert(b, dup);
        ASSERT_EQ(st, DS_ERR_EXIST, "random insert duplicate => DS_ERR_EXIST");
        free(dup);
      }
    } else {
      if (present[key]) {
        int free_before = g_free_count;
        ds_status_t st = ds_bst_remove(b, &key, counted_free);
        ASSERT_EQ(st, DS_OK, "random remove existing => DS_OK");
        ASSERT_EQ(g_free_count, free_before + 1, "random remove frees one");
        present[key] = 0;
      } else {
        ds_status_t st = ds_bst_remove(b, &key, NULL);
        ASSERT_EQ(st, DS_ERR_NOT_FOUND, "random remove missing => DS_ERR_NOT_FOUND");
      }
    }

    /* inorder must remain strictly increasing */
    out_reset();
    ds_bst_traverse_inorder(b, visit_collect);
    assert_sorted_strict_increasing(g_out, g_out_n, "inorder strictly increasing (random)");

    /* size should equal count(present) */
    size_t cnt = 0;
    for (int i = 0; i < MAXK; i++) if (present[i]) cnt++;
    ASSERT_EQ(ds_bst_size(b), cnt, "size matches reference count");
  }

  ds_bst_destroy(b, counted_free);
}

/* ===================== main ===================== */

int main() {
  test_case_t tests[] = {
    {"bst_create_basic", test_bst_create_basic},
    {"bst_insert_search_min_max_inorder", test_bst_insert_search_min_max_inorder},
    {"bst_insert_duplicate_key", test_bst_insert_duplicate_key},
    {"bst_remove_leaf_one_child_two_children", test_bst_remove_leaf_one_child_two_children},
    {"bst_remove_errors", test_bst_remove_errors},
    {"bst_random_insert_delete_inorder_sorted", test_bst_random_insert_delete_inorder_sorted},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
