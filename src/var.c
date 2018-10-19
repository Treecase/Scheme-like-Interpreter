/*
 * Variables
 *
 */

#include "data.h"
#include "var.h"
#include "token.h"

#include <stdlib.h>
#include <string.h>



/* vardup: duplicate a variable */
Var vardup (Var v)
{
    Var new = v;

    switch (v.type)
    {
    case VAR_STRING:
        new.str = stringdup (v.str);
        break;

    case VAR_SYMBOL:
        new.sym = stringdup (v.sym);
        break;

    case VAR_LIST:
        new.list.len  = v.list.len;
        new.list.data = malloc (new.list.len*sizeof(*new.list.data));
        memcpy (new.list.data, v.list.data,
                new.list.len * sizeof(*new.list.data));
        break;

    case VAR_FUNCTION:
        if (v.fn.type == FN_LISPFN)
        {
            new.fn.fn.body.data = malloc (new.fn.fn.body.len*
                                          sizeof(*new.fn.fn.body.data));

            for (size_t i = 0; i < v.fn.fn.body.len; ++i)
            {   new.fn.fn.body.data[i] = vardup (v.fn.fn.body.data[i]);
            }
        }
        break;

    case VAR_ERROR:
        new.err.flavour = strdup (v.err.flavour);
        break;

    /* these don't alloc memory */
    case VAR_NUMBER:
        break;
    }
    return new;
}

/* free_var: free variable memory */
void free_var (Var v)
{   switch (v.type)
    {
    case VAR_STRING:
        free_string (v.str);
        break;
    case VAR_SYMBOL:
        free_string (v.sym);
        break;

    case VAR_LIST:
        for (size_t i = 0; i < v.list.len; ++i)
        {   free_var (v.list.data[i]);
        }
        free (v.list.data);
        break;

    case VAR_FUNCTION:
        if (v.fn.type == FN_LISPFN)
        {   for (size_t i = 0; i < v.fn.fn.body.len; ++i)
            {   free_var (v.fn.fn.body.data[i]);
            }
            free (v.fn.fn.body.data);
            free_env (v.fn.fn.env);
        }
        break;

    case VAR_ERROR:
        free (v.err.flavour);
        break;

    /* these don't alloc memory */
    case VAR_NUMBER:
        break;
    }
}

