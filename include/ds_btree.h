#ifndef DS_BTREE_H
#define DS_BTREE_H

#include "ds_common.h"

/**
 * @brief Opaque binary tree node type.
 */
typedef struct ds_btree_node ds_btree_node_t;

/**
 * @brief Opaque list type.
 *
 * The internal structure is hidden from users.
 * All operations must be performed through the provided API.
 */
typedef struct ds_btree ds_btree_t;

/**
 * @brief Create a binary tree.
 *
 * @return Pointer to a new binary tree on success, or NULL on failure.
 */
ds_btree_t *ds_btree_create();

/**
 * @brief Destroy a binary tree and optionally free its elements.
 *
 * @param tree       Pointer to the binary tree.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the binary tree itself is freed.
 */
void ds_btree_destroy(ds_btree_t *tree, ds_free_f free_func);

/**
 * @brief Clear all nodes but keep the tree object.
 *
 * @param tree       Pointer to the list.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each element.
 *                   - If NULL, element pointers are discarded without
 *                     freeing the associated memory.
 */
void ds_btree_clear(ds_btree_t *tree, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_btree_size(const ds_btree_t *tree);

/**
 * @brief Get the height of binary tree.
 */
size_t ds_btree_height(const ds_btree_t *tree); 

/**
 * @brief Create a standalone node (not attached to tree yet).
 */
ds_btree_node_t *ds_btree_node_create(void *data);

/**
 * @brief Set the root of the tree. If root already exists, returns error. 
 */
ds_status_t ds_btree_set_root(ds_btree_t *tree, ds_btree_node_t *node);

/**
 * @brief Get the root node.
 */
ds_btree_node_t *ds_btree_root(const ds_btree_t *tree);

/**
 * @brief Attach a single node to the left/right of parent.
 *
 * The size of the tree increases by 1.
 * If `node` has children, the tree size will be incorrect!
 *
 * @param tree     The destination tree.
 * @param parent  Target node in the tree.
 * @param node    The node to attach.
 */
ds_status_t ds_btree_attach_node_left(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_node_t *node);
ds_status_t ds_btree_attach_node_right(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_node_t *node);

/**
 * @brief Graft (attach) an entire subtree to the left/right of parent.
 *
 * The ownership of nodes in `subtree` is transferred to `tree`.
 * The `subtree` object becomes empty (root=NULL, size=0).
 *
 * @param tree     The destination tree.
 * @param parent   Target node in the destination tree.
 * @param subtree  The source tree to be attached. 
 */
ds_status_t ds_btree_attach_tree_left(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_t *subtree);
ds_status_t ds_btree_attach_tree_right(ds_btree_t *tree, ds_btree_node_t *parent, ds_btree_t *subtree);

/* 
 * @brief Detach child (returns the detached node, does NOT free it).
 */
ds_btree_node_t *ds_btree_detach_left(ds_btree_t *tree, ds_btree_node_t *parent);
ds_btree_node_t *ds_btree_detach_right(ds_btree_t *tree, ds_btree_node_t *parent);

/**
 * @brief Access data in node.
 */
void *ds_btree_node_get(ds_btree_node_t *node);

/* -------------------------------------------------------------------------
 * Traversal (Callback based)
 * -------------------------------------------------------------------------
 */

/**
 * @brief Pre-order: Root -> Left -> Right.
 */
void ds_btree_traverse_preorder(const ds_btree_t *tree, ds_visit_f visit);
void ds_btree_traverse_preorder_iterative(const ds_btree_t *tree, ds_visit_f visit);

/**
 * @brief In-order: Left -> Root -> Right.
 */
void ds_btree_traverse_inorder(const ds_btree_t *tree, ds_visit_f visit);
void ds_btree_traverse_inorder_iterative(const ds_btree_t *tree, ds_visit_f visit);

/**
 * @brief Post-order: Left -> Right -> Root.
 */
void ds_btree_traverse_postorder(const ds_btree_t *tree, ds_visit_f visit);
void ds_btree_traverse_postorder_iterative(const ds_btree_t *tree, ds_visit_f visit);

/**
 * @brief Level-order: Breadth First Search.
 */
void ds_btree_traverse_levelorder(const ds_btree_t *tree, ds_visit_f visit);

#endif // !DS_BTREE_H
