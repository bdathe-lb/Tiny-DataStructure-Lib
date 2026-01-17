#include "ds_btree.h"
#include "ds_common.h"
#include "ds_queue.h"
#include "ds_stack.h"
#include <stddef.h>
#include <stdlib.h>

struct ds_btree_node {
  void *data;
  struct ds_btree_node *left;
  struct ds_btree_node *right;
};

struct ds_btree {
  ds_btree_node_t *root;
  size_t size;
};

/**
 * @brief Create a binary tree.
 *
 * @return Pointer to a new binary tree on success, or NULL on failure.
 */
ds_btree_t *ds_btree_create() {
  ds_btree_t *tree = malloc(sizeof(ds_btree_t));
  if (!tree) return NULL;

  tree->size = 0;
  tree->root = NULL;

  return tree;
}

/**
 * @brief Destroy a binary tree and optionally free its elements.
 *
 * Use ono-recursive (iterative) post-order traverse (dual-stack method)
 * Post-order sequence: Left -> Right -> Root
 * When reversed:       Left -> Right -> Root
 * Therefore, a dual-stack approach can be adopted:
 *  - First stack:  Used for traversal control (visiting in the reversed order: Root -> Right -> Left)
 *  - Second stack: Stores the "reversed visit sequence"
 *
 * @param tree       Pointer to the binary tree.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the binary tree itself is freed.
 */
void ds_btree_destroy(ds_btree_t *tree, ds_free_f free_func) {
  // Check input parameters
  if (!tree) return;

  if (!tree->root) {
    free(tree);
    return;
  }

  // Helper stack for traversal control
  ds_stack_t *stack_traverse = ds_stack_create(tree->size);
  if (!stack_traverse) return;

  // Helper stack for Stores results
  ds_stack_t *stack_store = ds_stack_create(tree->size);
  if (!stack_store) {
    ds_stack_destroy(stack_traverse, free_func);
    return;
  }

  ds_btree_node_t *curr = tree->root;
  
  while (curr) {
    ds_stack_push(stack_store, curr);

    if (curr->left) {
      ds_stack_push(stack_traverse, curr->left);
    }

    if (curr->right) {
      curr = curr->right;
    } else if (!ds_stack_is_empty(stack_traverse)) {
      curr = ds_stack_pop(stack_traverse);
    } else {
      break;
    }
  }

  // Free nodes one by one
  while (!ds_stack_is_empty(stack_store)) {
    ds_btree_node_t *tmp = ds_stack_pop(stack_store);
    if (free_func) free_func(tmp->data);
    free(tmp);
  }

  free(tree);
  ds_stack_destroy(stack_traverse, NULL);
  ds_stack_destroy(stack_store, NULL);
}

/**
 * @brief Clear all nodes but keep the tree object.
 *
 * Iterative post-order traversal (single stack + last_visited).
 *
 * Idea:
 *   Simulate recursive post-order without explicit state.
 *   The stack stores the path from root to current node.
 *
 *   last_visited records the most recently visited node.
 *   When examining the stack top `u`:
 *     - If u has a right child that has NOT been visited yet
 *       (last_visited != u->right), then descend into u->right.
 *     - Otherwise, both subtrees of u are finished (or absent),
 *       so u can be visited and popped from the stack.
 *
 *   In short:
 *     last_visited == u->right  => just returned from right subtree,
 *                                  safe to visit u (post-order).
 *
 * @param tree       Pointer to the list.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_btree_clear(ds_btree_t *tree, ds_free_f free_func) {
  // Check input parameters
  if (!tree || !tree->root) return;

  // Helper stack
  ds_stack_t *stack = ds_stack_create(tree->size);
  if (!stack) return;

  // Core: The node that was last fully visited 
  ds_btree_node_t *last_visited = NULL;

  ds_btree_node_t *curr = tree->root; 
  do {
    while (curr) {
      ds_stack_push(stack, curr);
      curr = curr->left;
    }

    ds_btree_node_t *peek = ds_stack_top(stack);
    
    // Right subtree incomplete, switching to process the right subtree
    if (peek->right && last_visited != peek->right) {
      curr = peek->right;
    }
    // Right subtree does not exist or has been processed, may proceed to access peek 
    else if (!peek->right || (peek->right && last_visited == peek->right)) {
      last_visited = ds_stack_pop(stack);
      if (free_func) free_func(peek->data);
      free(peek);
    }
  } while (!ds_stack_is_empty(stack));

  ds_stack_destroy(stack, NULL);
  tree->root = NULL;
  tree->size = 0;
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_btree_size(const ds_btree_t *tree) {
  // Check input parameters
  if (!tree)
    return 0;
  
  return tree->size;
}

/**
 * @brief Get the height of binary tree.
 */
static size_t btree_height_node(const ds_btree_node_t *node) {
  if (!node)  return 0;

  size_t left_subtree_height = 0; 
  size_t right_subtree_height = 0; 

  if (node->left) left_subtree_height = btree_height_node(node->left);
  if (node->right) right_subtree_height = btree_height_node(node->right);

  return left_subtree_height > right_subtree_height ?
         left_subtree_height + 1 :
         right_subtree_height + 1;
}

size_t ds_btree_height(const ds_btree_t *tree) {
  // Check input parameters
  if (!tree) return 0;

  // Recursive
  return btree_height_node(tree->root);
}

/**
 * @brief Create a standalone node (not attached to tree yet).
 */
ds_btree_node_t *ds_btree_node_create(void *data) {
  // Check input parameters
  if (!data) return NULL;

  ds_btree_node_t *node = malloc(sizeof(ds_btree_node_t));
  if (!node) return NULL;

  node->data = data;
  node->left = NULL;
  node->right = NULL;

  return node;
}

/**
 * @brief Set the root of the tree. If root already exists, returns error. 
 */
ds_status_t ds_btree_set_root(ds_btree_t *tree, ds_btree_node_t *node) {
  // Check input parameters
  if (!tree) return DS_ERR_NULL;
  if (!node) return DS_ERR_ARG;

  if (tree->root) return DS_ERR_EXIST;

  tree->root = node;
  tree->size ++;

  return DS_OK;
}

/**
 * @brief Get the root node.
 */
ds_btree_node_t *ds_btree_root(const ds_btree_t *tree) {
  // Check input parameters
  if (!tree) return NULL;

  return tree->root;
}

/**
 * @brief Attach a single node to the left of parent.
 *
 * The size of the tree increases by 1.
 * If `node` has children, the tree size will be incorrect!
 *
 * @param tree     The destination tree.
 * @param parent  Target node in the tree.
 * @param node    The node to attach.
 */
ds_status_t ds_btree_attach_node_left(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_node_t *node) {
  // Check input parameters
  if (!tree) return DS_ERR_NULL;
  if (!parent || !node) return DS_ERR_ARG;

  if (parent->left) return DS_ERR_EXIST;

  parent->left = node;
  tree->size ++;

  return DS_OK;
}

/**
 * @brief Attach a single node to the right of parent.
 *
 * The size of the tree increases by 1.
 * If `node` has children, the tree size will be incorrect!
 *
 * @param tree     The destination tree.
 * @param parent  Target node in the tree.
 * @param node    The node to attach.
 */
ds_status_t ds_btree_attach_node_right(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_node_t *node) {
  // Check input parameters
  if (!tree) return DS_ERR_NULL;
  if (!parent || !node) return DS_ERR_ARG;

  if (parent->right) return DS_ERR_EXIST;

  parent->right = node;
  tree->size ++;

  return DS_OK;
}

/**
 * @brief Graft (attach) an entire subtree to the left of parent.
 *
 * The ownership of nodes in `subtree` is transferred to `tree`.
 * The `subtree` object becomes empty (root=NULL, size=0).
 *
 * @param tree     The destination tree.
 * @param parent   Target node in the destination tree.
 * @param subtree  The source tree to be attached. 
 */
ds_status_t ds_btree_attach_tree_left(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_t *subtree) {
  // Check input parameters
  if (!tree) return DS_ERR_NULL;
  if (!parent || !subtree) return DS_ERR_ARG;

  // Empty tree, do nothing
  if (subtree->root == NULL) return DS_OK; 

  if (parent->left) return DS_ERR_EXIST;

  parent->left = subtree->root;
  tree->size += subtree->size;

  subtree->root = NULL;
  subtree->size = 0;

  return DS_OK;
}

/**
 * @brief Graft (attach) an entire subtree to the right of parent.
 *
 * The ownership of nodes in `subtree` is transferred to `tree`.
 * The `subtree` object becomes empty (root=NULL, size=0).
 *
 * @param tree     The destination tree.
 * @param parent   Target node in the destination tree.
 * @param subtree  The source tree to be attached. 
 */
ds_status_t ds_btree_attach_tree_right(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_t *subtree) {
  // Check input parameters
  if (!tree) return DS_ERR_NULL;
  if (!parent || !subtree) return DS_ERR_ARG;

  // Empty tree, do nothing
  if (subtree->root == NULL) return DS_OK; 

  if (parent->right) return DS_ERR_EXIST;

  parent->right = subtree->root;
  tree->size += subtree->size;

  subtree->root = NULL;
  subtree->size = 0;

  return DS_OK;
}

/* 
 * @brief Detach child (returns the detached node, does NOT free it).
 */
ds_btree_node_t *ds_btree_detach_left(ds_btree_t *tree, ds_btree_node_t *parent) {
  // Check input parameters
  if (!tree || !parent) return NULL;

  ds_btree_node_t *left = parent->left;
  parent->left = NULL;

  tree->size --;

  return left;
}

/* 
 * @brief Detach child (returns the detached node, does NOT free it).
 */
ds_btree_node_t *ds_btree_detach_right(ds_btree_t *tree, ds_btree_node_t *parent) {
  // Check input parameters
  if (!tree || !parent) return NULL;

  ds_btree_node_t *right = parent->right;
  parent->right = NULL;

  tree->size --;

  return right;
}

/**
 * @brief Access data in node.
 */
void *ds_btree_node_get(ds_btree_node_t *node) {
  // Check input parameters
  if (!node) return NULL;

  return node->data;
}

/* -------------------------------------------------------------------------
 * Traversal Pre-order: Root -> Left -> Right.
 * -------------------------------------------------------------------------
 */
static void preorder_node(const ds_btree_node_t *node, ds_visit_f visit) {
  // Recursive termination condition
  if (!node) return;

  // Root
  visit(node->data);
  // Left
  preorder_node(node->left, visit);
  // Right
  preorder_node(node->right, visit);
}

void ds_btree_traverse_preorder(const ds_btree_t *tree, ds_visit_f visit) {
  // Check input parameters
  if (!tree || !tree->root || !visit) return;

  preorder_node(tree->root, visit);
}

void ds_btree_traverse_preorder_iterative(const ds_btree_t *tree, ds_visit_f visit) {
  // Check input parameters
  if (!tree || !tree->root || !visit) return;
   
  // In the recursive version, the CPU retains the return address after preorder(left) completes,
  // so it can proceed to execute preorder(right) for the right‑subtree traversal.
  //
  // In the non‑recursive (iterative) version, an explicit stack is used to remember
  // which node should be processed after returning from the left subtree.
  // Due to the LIFO nature of the stack, the right‑subtree node pushed later is guaranteed to be processed first.

  // Helper stack
  ds_stack_t *stack = ds_stack_create(tree->size);
  if (!stack) return;

  // DFS preorder
  ds_btree_node_t *curr = tree->root;
  
  while (curr) {
    visit(curr->data);

    if (curr->right) {
      ds_stack_push(stack, curr->right);
    }

    if (curr->left) {
      curr = curr->left;
    } else if (!ds_stack_is_empty(stack)) {
      curr = ds_stack_pop(stack);
    } else {
      break;
    }
  }

  ds_stack_destroy(stack, NULL);
}

/* -------------------------------------------------------------------------
 * Traversal In-order: Left -> Root -> Right.
 * -------------------------------------------------------------------------
 */
static void inorder_node(const ds_btree_node_t *node, ds_visit_f visit) {
  // Recursive termination condition
  if (!node) return;

  // Left
  inorder_node(node->left, visit);
  // Root
  visit(node->data);
  // Right
  inorder_node(node->right, visit);
}

void ds_btree_traverse_inorder(const ds_btree_t *tree, ds_visit_f visit) {
  // Check input parameters
  if (!tree || !tree->root || !visit) return;

  inorder_node(tree->root, visit);
}

void ds_btree_traverse_inorder_iterative(const ds_btree_t *tree, ds_visit_f visit) {
  // Check input parameters
  if (!tree || !tree->root || !visit) return;

  // Helper stack
  ds_stack_t *stack = ds_stack_create(tree->size);
  if (!stack)
    return;

  ds_btree_node_t *curr = tree->root;

  while (curr) {
    ds_stack_push(stack, curr);

    if (curr->left) {
      curr = curr->left;
    } else {
      curr = ds_stack_pop(stack);
      visit(curr->data);
      curr = curr->right;

      while (!curr) {
        if (ds_stack_is_empty(stack)) break;

        curr = ds_stack_pop(stack);
        visit(curr->data);
        curr = curr->right;
      }
    }
  }

  ds_stack_destroy(stack, NULL);
}

/* -------------------------------------------------------------------------
 * Traversal Post-order: Left -> Right -> Root.
 * -------------------------------------------------------------------------
 */
static void postorder_node(const ds_btree_node_t *node, ds_visit_f visit) {
  // Recursive termination condition
  if (!node) return;

  // Left
  postorder_node(node->left, visit);
  // Right
  postorder_node(node->right, visit);
  // Root
  visit(node->data);
}

void ds_btree_traverse_postorder(const ds_btree_t *tree, ds_visit_f visit) {
  // Check input parameters
  if (!tree || !tree->root || !visit) return;

  postorder_node(tree->root, visit);
}

/* Internal stack frame for iterative post-order traversal (single-stack) */
typedef enum {
  BTREE_POST_ENTER = 0,       // first time at node: before left subtree
  BTREE_POST_LEFT_DONE,       // left subtree finished: before right subtree
  BTREE_POST_DONE             // left & right finished: ready to visit node
} btree_post_state_t;

typedef struct {
  ds_btree_node_t   *node;    // current tree node
  btree_post_state_t state;   // traversal state of this node
} btree_post_frame_t;

void free_post_frame(void *f) {
  if (f) free(f);
}

void ds_btree_traverse_postorder_iterative(const ds_btree_t *tree, ds_visit_f visit) {
  // Check input parameters 
  if (!tree || !tree->root || !visit) return;

  // Helper stack
  ds_stack_t *stack = ds_stack_create(tree->size);
  if (!stack) return;

  btree_post_frame_t *f_root = malloc(sizeof(btree_post_frame_t));
  if (!f_root) {
    ds_stack_destroy(stack, free_post_frame);
    return;
  }
  f_root->node = tree->root;
  f_root->state = BTREE_POST_ENTER;
  ds_stack_push(stack, f_root);

  while (!ds_stack_is_empty(stack)) {
    btree_post_frame_t *f = (btree_post_frame_t *)ds_stack_top(stack);

    switch (f->state) {
      case BTREE_POST_ENTER: {
        f->state  = BTREE_POST_LEFT_DONE;
        if (f->node->left) {
          btree_post_frame_t *f_left = malloc(sizeof(btree_post_frame_t));
          if (!f_left) {
            ds_stack_destroy(stack, free_post_frame);
            return;
          }
          f_left->node = f->node->left;
          f_left->state = BTREE_POST_ENTER;
          ds_stack_push(stack, f_left);
        }
        break;
      }

      case BTREE_POST_LEFT_DONE: {
        f->state = BTREE_POST_DONE;
        if (f->node->right) {
          btree_post_frame_t *f_right = malloc(sizeof(btree_post_frame_t));
          if (!f_right) {
            ds_stack_destroy(stack, free_post_frame);
            return;
          }
          f_right->node = f->node->right;
          f_right->state = BTREE_POST_ENTER;
          ds_stack_push(stack, f_right);
        }
        break;
      }

      case BTREE_POST_DONE:
        visit(f->node->data);
        void *tmp = ds_stack_pop(stack);
        free_post_frame(tmp);
        break;

      default:
        break;
    }
  }

  ds_stack_destroy(stack, NULL);
}

/**
 * @brief Level-order: Breadth First Search.
 */
void ds_btree_traverse_levelorder(const ds_btree_t *tree, ds_visit_f visit) {
  // Check input parameters
  if (!tree || !tree->root || !visit) return;

  // Helper queue
  ds_queue_t *queue = ds_queue_create(tree->size);
  if (!queue) return;

  ds_queue_push(queue, tree->root);

  while (!ds_queue_is_empty(queue)) {
    ds_btree_node_t *curr = ds_queue_pop(queue);
    visit(curr->data);

    if (curr->left)  ds_queue_push(queue, curr->left);
    if (curr->right) ds_queue_push(queue, curr->right);
  }

  ds_queue_destroy(queue, NULL);
}
