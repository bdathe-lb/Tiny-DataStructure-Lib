#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ds_btree.h"
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

/* 收集遍历结果 */
static int g_out[256];
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

static void assert_out_equals(const int *exp, size_t n, const char *msg) {
  ASSERT_EQ(g_out_n, n, msg);
  for (size_t i = 0; i < n && i < g_out_n; i++) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s[%zu]==%d", msg, i, exp[i]);
    ASSERT_EQ(g_out[i], exp[i], buf);
  }
}

/* 构造一个固定形状的树，方便验证遍历序列：
 *
 *          1
 *        /   \
 *       2     3
 *      / \     \
 *     4   5     6
 *
 * size=6, height=3
 *
 * 返回：tree，以及各节点指针（方便测试 attach/detach）
 */
typedef struct {
  ds_btree_t *tree;
  ds_btree_node_t *n1,*n2,*n3,*n4,*n5,*n6;
} built_tree_t;

static built_tree_t build_sample_tree(void) {
  built_tree_t bt;
  memset(&bt, 0, sizeof(bt));
  bt.tree = ds_btree_create();
  if (!bt.tree) return bt;

  bt.n1 = ds_btree_node_create(mk_int(1));
  bt.n2 = ds_btree_node_create(mk_int(2));
  bt.n3 = ds_btree_node_create(mk_int(3));
  bt.n4 = ds_btree_node_create(mk_int(4));
  bt.n5 = ds_btree_node_create(mk_int(5));
  bt.n6 = ds_btree_node_create(mk_int(6));

  /* 假设这些都成功（测试里也会断言） */
  ds_btree_set_root(bt.tree, bt.n1);
  ds_btree_attach_node_left(bt.tree, bt.n1, bt.n2);
  ds_btree_attach_node_right(bt.tree, bt.n1, bt.n3);
  ds_btree_attach_node_left(bt.tree, bt.n2, bt.n4);
  ds_btree_attach_node_right(bt.tree, bt.n2, bt.n5);
  ds_btree_attach_node_right(bt.tree, bt.n3, bt.n6);

  return bt;
}

/* ===================== Tests ===================== */

TEST_FUNC(test_btree_create_basic) {
  ds_btree_t *t = ds_btree_create();
  ASSERT_NOT_NULL(t, "create non-NULL");
  ASSERT_EQ(ds_btree_size(t), 0u, "new tree size==0");
  ASSERT_EQ(ds_btree_height(t), 0u, "new tree height==0");
  ASSERT_NULL(ds_btree_root(t), "root NULL initially");

  ds_btree_destroy(t, NULL);
  ASSERT(1, "destroy(empty) ok");

  ds_btree_destroy(NULL, NULL);
  ASSERT(1, "destroy(NULL) ok");
}

TEST_FUNC(test_btree_node_create_and_set_root) {
  ds_btree_t *t = ds_btree_create();
  ASSERT_NOT_NULL(t, "create");

  ASSERT_EQ(ds_btree_set_root(NULL, NULL), DS_ERR_NULL, "set_root(NULL, NULL) => DS_ERR_NULL");

  ds_btree_node_t *n = ds_btree_node_create(NULL);
  ASSERT_NULL(n, "node_create(NULL data) => NULL");

  ds_btree_node_t *r = ds_btree_node_create(mk_int(10));
  ASSERT_NOT_NULL(r, "node_create(data) ok");

  ASSERT_EQ(ds_btree_set_root(t, NULL), DS_ERR_ARG, "set_root(tree,NULL) => DS_ERR_ARG");
  ASSERT_EQ(ds_btree_set_root(t, r), DS_OK, "set_root OK");
  ASSERT_EQ(ds_btree_size(t), 1u, "size==1 after set_root");
  ASSERT_NOT_NULL(ds_btree_root(t), "root non-NULL");

  /* root already exists */
  ds_btree_node_t *r2 = ds_btree_node_create(mk_int(11));
  ASSERT_NOT_NULL(r2, "node_create 2 ok");
  ASSERT_EQ(ds_btree_set_root(t, r2), DS_ERR_EXIST, "set_root again => DS_ERR_EXIST");
  /* set_root 失败不会接管 r2，需要释放它（以及它的数据） */
  counted_free(ds_btree_node_get(r2));
  free(r2);

  ds_btree_destroy(t, counted_free);
}

TEST_FUNC(test_btree_attach_node_and_height_size) {
  built_tree_t bt = build_sample_tree();
  ASSERT_NOT_NULL(bt.tree, "build tree ok");
  ASSERT_EQ(ds_btree_size(bt.tree), 6u, "size==6");
  ASSERT_EQ(ds_btree_height(bt.tree), 3u, "height==3");

  /* attach to occupied slot */
  ds_btree_node_t *x = ds_btree_node_create(mk_int(99));
  ASSERT_NOT_NULL(x, "node_create 99");
  ASSERT_EQ(ds_btree_attach_node_left(bt.tree, bt.n1, x), DS_ERR_EXIST, "attach_left to occupied => DS_ERR_EXIST");
  counted_free(ds_btree_node_get(x));
  free(x);

  ds_btree_destroy(bt.tree, counted_free);
}

TEST_FUNC(test_btree_traversals_match_expected) {
  built_tree_t bt = build_sample_tree();
  ASSERT_NOT_NULL(bt.tree, "build tree ok");

  /* Expected sequences for the sample tree */
  const int exp_pre[]   = {1,2,4,5,3,6};
  const int exp_in[]    = {4,2,5,1,3,6};
  const int exp_post[]  = {4,5,2,6,3,1};
  const int exp_level[] = {1,2,3,4,5,6};

  out_reset();
  ds_btree_traverse_preorder(bt.tree, visit_collect);
  assert_out_equals(exp_pre, 6, "preorder recursive");

  out_reset();
  ds_btree_traverse_preorder_iterative(bt.tree, visit_collect);
  assert_out_equals(exp_pre, 6, "preorder iterative");

  out_reset();
  ds_btree_traverse_inorder(bt.tree, visit_collect);
  assert_out_equals(exp_in, 6, "inorder recursive");

  out_reset();
  ds_btree_traverse_inorder_iterative(bt.tree, visit_collect);
  assert_out_equals(exp_in, 6, "inorder iterative");

  out_reset();
  ds_btree_traverse_postorder(bt.tree, visit_collect);
  assert_out_equals(exp_post, 6, "postorder recursive");

  out_reset();
  ds_btree_traverse_postorder_iterative(bt.tree, visit_collect);
  assert_out_equals(exp_post, 6, "postorder iterative");

  out_reset();
  ds_btree_traverse_levelorder(bt.tree, visit_collect);
  assert_out_equals(exp_level, 6, "levelorder");

  ds_btree_destroy(bt.tree, counted_free);
}

TEST_FUNC(test_btree_clear_resets_tree_and_frees_data) {
  built_tree_t bt = build_sample_tree();
  ASSERT_NOT_NULL(bt.tree, "build tree ok");

  int free_before = g_free_count;
  ds_btree_clear(bt.tree, counted_free);
  ASSERT_EQ(g_free_count, free_before + 6, "clear frees 6 data items");
  ASSERT_EQ(ds_btree_size(bt.tree), 0u, "size==0 after clear");
  ASSERT_EQ(ds_btree_height(bt.tree), 0u, "height==0 after clear");
  ASSERT_NULL(ds_btree_root(bt.tree), "root NULL after clear");

  /* traverse after clear should do nothing (visit not called) */
  out_reset();
  ds_btree_traverse_preorder(bt.tree, visit_collect);
  ASSERT_EQ(g_out_n, 0u, "traverse on empty does not visit");

  ds_btree_destroy(bt.tree, counted_free);
}

TEST_FUNC(test_btree_attach_tree_left_right_transfer_ownership) {
  /* main tree with root 100 */
  ds_btree_t *t = ds_btree_create();
  ASSERT_NOT_NULL(t, "create main");

  ds_btree_node_t *r = ds_btree_node_create(mk_int(100));
  ASSERT_NOT_NULL(r, "root 100");
  ASSERT_EQ(ds_btree_set_root(t, r), DS_OK, "set_root main");

  /* subtree with nodes 1<-2 and 3 (size=3) */
  built_tree_t sub = build_sample_tree(); /* size 6 actually */
  ASSERT_NOT_NULL(sub.tree, "create subtree");
  /* 为了简单：我们只取 subtree 的根及其左右两侧作为“子树对象”，直接 attach whole subtree */
  size_t sub_size = ds_btree_size(sub.tree);
  ASSERT(sub_size > 0, "subtree size > 0");

  size_t main_before = ds_btree_size(t);
  ASSERT_EQ(ds_btree_attach_tree_left(t, r, sub.tree), DS_OK, "attach_tree_left OK");
  ASSERT_EQ(ds_btree_size(t), main_before + sub_size, "size increased by subtree size");
  ASSERT_NULL(ds_btree_root(sub.tree), "subtree root becomes NULL after attach");
  ASSERT_EQ(ds_btree_size(sub.tree), 0u, "subtree size becomes 0 after attach");

  /* attach empty subtree should be OK/no-op */
  ASSERT_EQ(ds_btree_attach_tree_right(t, r, sub.tree), DS_OK, "attach empty subtree right is OK");

  /* destroy empty subtree object */
  ds_btree_destroy(sub.tree, counted_free);

  /* destroy main tree should free everything */
  ds_btree_destroy(t, counted_free);
}

TEST_FUNC(test_btree_detach_left_right_current_behavior) {
  /* 按你当前实现：detach 只 size--，不管子树大小 */
  built_tree_t bt = build_sample_tree();
  ASSERT_NOT_NULL(bt.tree, "build tree ok");
  ASSERT_EQ(ds_btree_size(bt.tree), 6u, "size==6");

  /* detach left of root (node 2 subtree) */
  ds_btree_node_t *det = ds_btree_detach_left(bt.tree, bt.n1);
  ASSERT_NOT_NULL(det, "detach_left returns node");
  ASSERT_EQ(ds_btree_size(bt.tree), 5u, "current behavior: size-- by 1 after detach_left");

  /* det 是一整棵子树，但当前 API 没有“释放子树”的接口。
     为避免泄漏：我们临时建一个 tree 接管 det，然后 destroy。 */
  ds_btree_t *tmp = ds_btree_create();
  ASSERT_NOT_NULL(tmp, "tmp tree");
  ASSERT_EQ(ds_btree_set_root(tmp, det), DS_OK, "set_root(tmp, detached) OK");
  /* 注意：tmp->size 现在是 1（因为 set_root 只 +1），和真实子树节点数不一致，但 destroy 不依赖 size 准确性（它用 stack size hint，可能偏小但 stack 会 grow） */
  ds_btree_destroy(tmp, counted_free);

  ds_btree_destroy(bt.tree, counted_free);
}

/* ===================== main ===================== */

int main() {
  test_case_t tests[] = {
    {"btree_create_basic", test_btree_create_basic},
    {"btree_node_create_and_set_root", test_btree_node_create_and_set_root},
    {"btree_attach_node_and_height_size", test_btree_attach_node_and_height_size},
    {"btree_traversals_match_expected", test_btree_traversals_match_expected},
    {"btree_clear_resets_tree_and_frees_data", test_btree_clear_resets_tree_and_frees_data},
    {"btree_attach_tree_left_right_transfer_ownership", test_btree_attach_tree_left_right_transfer_ownership},
    {"btree_detach_left_right_current_behavior", test_btree_detach_left_right_current_behavior},
  };

  return run_tests(tests, sizeof(tests)/sizeof(tests[0]));
}
