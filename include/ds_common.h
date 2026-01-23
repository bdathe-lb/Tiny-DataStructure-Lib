
#ifndef DS_COMMON_H
#define DS_COMMON_H

#include <stddef.h>   // for `size_t`
#include <stdbool.h>  // for `bool`

/*
 * Status Codes
 * Unified status codes used for function return values.
 */
typedef enum {
    DS_OK = 0,                      // Operation successful

    /* Parameter/Usage Errors */
    DS_ERR_NULL   = -1,             // NULL pointer passed (vec==NULL / out==NULL, etc.)
    DS_ERR_ARG    = -2,             // Illegal parameter value (element==NULL, invalid capacity_hint, etc.)

    /* Container State/Boundary Errors */
    DS_ERR_BOUNDS = -3,             // Out of bounds: index >= size, or index > size(for insertion), etc.
    DS_ERR_EMPTY  = -4,             // Empty container (e.g., pop_front on empty, as required by API)
    DS_ERR_EXIST  = -5,             // A resource already exists
    DS_ERR_NOT_FOUND = -6,

    /* Resource/System Errors */
    DS_ERR_MEM    = -7,             // Memory allocation failed
} ds_status_t;

/*
 * Callback Function Types
 * Used to decouple data ownership from operational logic.
 */

// Memory release callback function.
typedef void (*ds_free_f)(void *data);
// Element size comparison callback function.
// Return value < 0: a comes before b.  
// Return value = 0: a is equal to b.  
// Return value > 0: a comes after b.
typedef int (*ds_compare_f)(const void *a, const void *b);
// Element visit callback function. 
typedef void (*ds_visit_f)(void *data);

#endif // !DS_COMMON_H
