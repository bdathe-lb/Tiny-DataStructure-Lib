#include "ds_bst.h"
#include "ds_stack.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct ds_bst_node {
  void *data;
  struct ds_bst_node *left;
  struct ds_bst_node *right;
} ds_bst_node_t;

struct ds_bst {
  ds_bst_node_t *root;
  size_t size;
  ds_compare_f compare;
};

/**
 * @brief Create a standalone node (not attached to tree yet).
 */
static ds_bst_node_t *ds_bst_node_create(void *data) {
  if (!data) return NULL;

  ds_bst_node_t *node = malloc(sizeof(ds_bst_node_t));
  if (!node) return NULL;

  node->data = data;
  node->left = NULL;
  node->right = NULL;

  return node;
}

/**
 * @brief Create a new binary search tree.
 *
 * @param compare  Function used to compare elements. 
 * Required to maintain the order of the tree.
 * - Returns < 0 if a < b
 * - Returns 0 if a == b
 * - Returns > 0 if a > b
 *
 * @return Pointer to a new BST on success, or NULL on failure.
 */
ds_bst_t *ds_bst_create(ds_compare_f compare) {
  // Check input parameters
  if (!compare) return NULL;

  ds_bst_t *bst = malloc(sizeof(ds_bst_t));
  if (!bst) return NULL;

  bst->root = NULL;
  bst->size = 0;
  bst->compare = compare;

  return bst;
}

/**
 * @brief Destroy a binary search tree and optionally free its elements.
 *
 * @param tree       Pointer to the binary search tree.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the binary search tree itself is freed.
 */
void ds_bst_destroy(ds_bst_t *bst, ds_free_f free_func) {
  // Check input parameters
  if (!bst) return;

  if (!bst->root) {
    free(bst);
    return;
  }

  // Helper stack for traversal control
  ds_stack_t *stack_traverse = ds_stack_create(bst->size);
  if (!stack_traverse) return;

  // Helper stack for Stores results
  ds_stack_t *stack_store = ds_stack_create(bst->size);
  if (!stack_store) {
    ds_stack_destroy(stack_traverse, NULL);
    return;
  }

  ds_bst_node_t *curr = bst->root;
  
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
    ds_bst_node_t *tmp = ds_stack_pop(stack_store);
    if (free_func) free_func(tmp->data);
    free(tmp);
  }

  free(bst);
  ds_stack_destroy(stack_traverse, NULL);
  ds_stack_destroy(stack_store, NULL);
}

/**
 * @brief Get the current number of elements.
 */
size_t ds_bst_size(const ds_bst_t *bst) {
  if (!bst) return 0;
  
  return bst->size;
}

/**
 * @brief Insert a new element into the BST.
 *
 * @param bst   Pointer to the BST.
 * @param data  Pointer to the data to insert.
 *
 * @return 
 * - DS_OK:      On success.
 * - DS_ERR:     If the data already exists (Duplicate keys are not allowed).
 * - DS_ERR_MEM: If memory allocation fails.
 */
ds_status_t ds_bst_insert(ds_bst_t *bst, void *data) {
  // Check input parameters
  if (!bst) return DS_ERR_NULL;
  if (!data) return DS_ERR_ARG;

  // Empty tree: new node becomes root
  if (!bst->root) {
    ds_bst_node_t *node = ds_bst_node_create(data);
    if (!node) return DS_ERR_MEM;

    bst->root = node;
    bst->size ++;

    return DS_OK;
  }

  // Locate the insertion point
  ds_bst_node_t **link = &bst->root;
  ds_bst_node_t *curr = NULL;

  while (*link) {
    curr = *link;
    int cmp = bst->compare(data, curr->data);
    if (cmp < 0)      link = &curr->left;
    else if (cmp > 0) link = &curr->right;
    else return DS_ERR_EXIST;  // Duplicate key not allowed
  }

  ds_bst_node_t *node = ds_bst_node_create(data);
  if (!node) return DS_ERR_MEM;

  // Insertion
  *link = node;

  bst->size ++;
  return DS_OK;
}

/**
 * @brief Search for data matching a key.
 *
 * @param bst  Pointer to the BST.
 * @param key  Pointer to a dummy object or key used for comparison.
 *
 * @return Pointer to the stored data if found, NULL otherwise.
 */
void *ds_bst_search(const ds_bst_t *bst, const void *key) {
  // Check input parameters
  if (!bst || !bst->root || !key) return NULL;

  const ds_bst_node_t *curr = bst->root;
  while (curr) {
    int cmp = bst->compare(key, curr->data);
    
    if (cmp < 0) {
      curr = curr->left;
    } else if (cmp > 0) {
      curr = curr->right;
    } else {
      // Found it
      return curr->data;
    }
  }

  return NULL;
}

/**
 * @brief Remove the node matching the key.
 *
 * @param bst        Pointer to the BST.
 * @param key        Pointer to the key to identify the node.
 * @param free_func  Optional destructor. 
 * If found, this is called on the data being removed.
 *
 * @return 
 * - DS_OK:    If the node was found and removed.
 * - DS_ERR_*: If the key was not found.
 */
ds_status_t ds_bst_remove(ds_bst_t *bst, const void *key, ds_free_f free_func) {
  // Check input parameters
  if (!bst) return DS_ERR_NULL;
  if (!bst->root) return DS_ERR_NOT_FOUND;
  if (!key) return DS_ERR_ARG;

  ds_bst_node_t **link = &bst->root;
  ds_bst_node_t *curr = NULL;

  while (*link) {
    curr = *link;
    int cmp = bst->compare(key, curr->data);
    if (cmp < 0) link = &curr->left;
    else if (cmp > 0) link = &curr->right;
    else break;
  }

  if (!*link) return DS_ERR_NOT_FOUND;

  // Case 1: The node to be deleted is a leaf node
  if (!curr->left && !curr->right) {
    // Delete directly
    *link = NULL;
  } 
  // Case 2: The node to delete has only a left subtree
  else if (curr->left && !curr->right) {
    // Replace the node to be deleted with the root node of its left subtree
    *link = curr->left;
  }
  // Case 3: The node to delete has only a right subtree
  else if (!curr->left && curr->right) {
    // Replace the node to be deleted with the root node of its right subtree
    *link = curr->right;
  }
  // Case 4: The node to be deleted has both a left subtree and a right subtree
  else {
    // 1)  Find the maximum node in the left subtree to Replace
    //     the node to be deleted
    ds_bst_node_t **pred_link = &curr->left;
    ds_bst_node_t *pred = *pred_link;
    while (pred->right) {
      pred_link = &pred->right;
      pred = *pred_link;
    }
    
    // 2) The maximum node in the left subtree has no right child 
    // (if it did, that right child would be larger)
    // If it has a left child, that left child takes its place
    *pred_link = pred->left;
    
    // 3) Replace the node to be deleted with the maximum node
    *link = pred;
    pred->left = curr->left;
    pred->right = curr->right;
  }

  // Free
  if (free_func) free_func(curr->data);
  free(curr);

  bst->size --;

  return DS_OK;
}

/**
 * @brief Get the minimum element (the "left-most" node).
 *
 * @param bst Pointer to the BST.
 *
 * @return Pointer to the minimum data, or NULL if tree is empty.
 */
void *ds_bst_min(const ds_bst_t *bst) {
  // Check input parameters
  if (!bst || !bst->root) return NULL;

  ds_bst_node_t *curr = bst->root;
  while (curr->left) curr = curr->left;

  return curr->data;
}

/**
 * @brief Get the maximum element (the "right-most" node).
 *
 * @param bst Pointer to the BST.
 * @return Pointer to the maximum data, or NULL if tree is empty.
 */
void *ds_bst_max(const ds_bst_t *bst) {
  // Check input parameters
  if (!bst || !bst->root) return NULL;

  ds_bst_node_t *curr = bst->root;
  while (curr->right) curr = curr->right;

  return curr->data;
}

/**
 * @brief Perform an In-order traversal.
 * * In a BST, in-order traversal guarantees visiting elements 
 * in strictly increasing order (Sorted).
 *
 * @param bst    Pointer to the BST.
 * @param visit  Callback function to process each element.
 */
static void inorder_node(const ds_bst_node_t *node, ds_visit_f visit) {
  if (!node) return;

  inorder_node(node->left, visit);
  visit(node->data);
  inorder_node(node->right, visit);
}

void ds_bst_traverse_inorder(const ds_bst_t *bst, ds_visit_f visit) {
  // Check input parameters
  if (!bst) return;

  inorder_node(bst->root, visit);
}

