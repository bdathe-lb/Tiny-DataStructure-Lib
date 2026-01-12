
#ifndef DS_COMMON_H
#define DS_COMMON_H

#include <stddef.h>   // for `size_t`
#include <stdbool.h>  // for `bool`

/*
 * Status Codes
 * Unified status codes used for function return values.
 */
typedef enum {
  DS_OK = 0,           // Operation successful
  DS_ERR = -1,         // General error
  DS_ERR_MEM = -2,     // Memory allocation failed
  DS_ERR_BOUNDS = -3   // Index out of bounds / Invalid parameter
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
//Element printing/debugging callback function. 
typedef void (*ds_print_f)(void *data);

#endif // !DS_COMMON_H
