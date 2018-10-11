/*
 * Variables
 *
 */

#include "data.h"
#include "var.h"
#include "token.h"

#include <stdlib.h>



/* free_var: free variable memory */
void free_var (Var v)
{   switch (v.type)
    {
    case VAR_STRING:
        free_string (v.str);
        break;

    case VAR_LIST:
        for (size_t i = 0; i < v.list.len; ++i)
        {   free_var (v.list.data[i]);
        }
        free (v.list.data);
        break;

    case VAR_FUNCTION:
        if (v.fn.type == FN_LISPFN)
        {   free_tokens (v.fn.fn.body);
        }
        break;

    /* these don't alloc memory */
    case VAR_ERROR:
    case VAR_NUMBER:
        break;
    }
}

