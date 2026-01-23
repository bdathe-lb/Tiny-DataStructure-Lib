#ifndef DS_BST_H
#define DS_BST_H

#include "ds_common.h"

/** -------------------------------------------------------------------------
 * 🧠 ARCHITECTURE: The "Key" vs "Data" Model
 * -------------------------------------------------------------------------
 *
 * This library stores generic `void *` pointers (Data). 
 * It does not strictly separate "Key" and "Value" in the API signatures.
 * Instead, the distinction is logical and defined by your `ds_compare_f`.
 *
 * 1. Data (Key + Payload): The complete object stored in the tree.
 * 2. Key: The specific field(s) within the Data used for sorting/comparison.
 *
 * [ Usage Pattern: Dummy Key ]
 * When calling `ds_bst_search()` or `ds_bst_remove()`, you do NOT need to 
 * provide the original pointer address. You can construct a temporary 
 * "Dummy Object" (on the stack) where ONLY the Key fields are populated, 
 * and pass its address.
 *
 * Example:
 * typedef struct { int id; char name[64]; } user_t;
 * // Compare function only looks at 'id' (The Key)
 * int compare_user(const void *a, const void *b) {
 *   return ((user_t*)a)->id - ((user_t*)b)->id;
 * }
 *
 * // Search for ID 1001 (using a Dummy Object)
 * user_t key_only = { .id = 1001 }; 
 * user_t *result = ds_bst_search(tree, &key_only);
 * -------------------------------------------------------------------------
 */

/**
 * @brief Opaque Binary Search Tree type.
 *
 * The internal structure is hidden from users.
 * A BST maintains an ordered structure where for any node:
 * left_child < node < right_child
 */
typedef struct ds_bst ds_bst_t;

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
ds_bst_t *ds_bst_create(ds_compare_f compare);

/**
 * @brief Destroy a binary search tree and optionally free its elements.
 *
 * @param tree       Pointer to the binary search tree.
 * @param free_func  Optional element destructor.
 *                   - If non-NULL, it is called for each stored element.
 *                   - If NULL, only the binary search tree itself is freed.
 */
void ds_bst_destroy(ds_bst_t *bst, ds_free_f free_func);

/**
 * @brief Get the current number of elements.
 */
size_t ds_bst_size(const ds_bst_t *bst);

/**
 * @brief Insert a new element into the BST.
 *
 * @param bst   Pointer to the BST.
 * @param data  Pointer to the data to insert.
 *
 * @return 
 * - DS_OK:    On success.
 * - DS_ERR_*: On failure.
 */
ds_status_t ds_bst_insert(ds_bst_t *bst, void *data);

/**
 * @brief Search for data matching a key.
 *
 * @param bst  Pointer to the BST.
 * @param key  Pointer to a dummy object or key used for comparison.
 *
 * @return Pointer to the stored data if found, NULL otherwise.
 */
void *ds_bst_search(const ds_bst_t *bst, const void *key);

/**
 * @brief Remove the node matching the key.
 *
 * @param bst        Pointer to the BST.
 * @param key        Pointer to the key to identify the node.
 * @param free_func  Optional destructor. 
 * If found, this is called on the data being removed.
 *
 * @return 
 * - DS_OK:      On success.
 * - DS_ERR:     If the data already exists (Duplicate keys are not allowed).
 * - DS_ERR_MEM: If memory allocation fails.
 */
ds_status_t ds_bst_remove(ds_bst_t *bst, const void *key, ds_free_f free_func);

/**
 * @brief Get the minimum element (the "left-most" node).
 *
 * @param bst Pointer to the BST.
 *
 * @return Pointer to the minimum data, or NULL if tree is empty.
 */
void *ds_bst_min(const ds_bst_t *bst);

/**
 * @brief Get the maximum element (the "right-most" node).
 *
 * @param bst Pointer to the BST.
 * @return Pointer to the maximum data, or NULL if tree is empty.
 */
void *ds_bst_max(const ds_bst_t *bst);

/**
 * @brief Perform an In-order traversal.
 * * In a BST, in-order traversal guarantees visiting elements 
 * in strictly increasing order (Sorted).
 *
 * @param bst    Pointer to the BST.
 * @param visit  Callback function to process each element.
 */
void ds_bst_traverse_inorder(const ds_bst_t *bst, ds_visit_f visit);

#endif // !DS_BST_H
