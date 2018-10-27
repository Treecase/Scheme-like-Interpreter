/*
 * Variables
 *
 */

#include "data.h"
#include "var.h"
#include "token.h"

#include <stdlib.h>
#include <string.h>



/* new_var: create a new Var */
Var *new_var (enum VarType t)
{
    Var *ref  = GC_malloc (sizeof(*ref));
    ref->type = t;

    return ref;
}

/* duplicate_var: duplicate v */
Var *duplicate_var (Var *v)
{
    Var *new = new_var (v->type);

    switch (v->type)
    {
    case VAR_ERROR:
        new->err = duplicate_err (v->err);
        break;
    case VAR_STRING:
        new->str = stringdup (v->str);
        break;
    case VAR_LIST:
        new->list = duplicate_list (v->list);
        break;
    case VAR_SYMBOL:
        new->sym = stringdup (v->sym);
        break;
    case VAR_IDENTIFIER:
        new->id = stringdup (v->id);
        break;

    case VAR_FUNCTION:
        new->fn = duplicate_fn (v->fn);
        break;

    case VAR_NUMBER:
        new->number = v->number;
        break;
    case VAR_BOOLEAN:
        new->boolean = v->boolean;
        break;

    /* non-valued types */
    case VAR_UNDEFINED:
    case VAR_EMPTY:
        break;
    }

    return new;
}



/* duplicate_list: duplicate l */
List duplicate_list (List l)
{
    List new;
    new.len = l.len;
    new.data = GC_malloc (new.len * sizeof(*new.data));

    for (size_t i = 0; i < new.len; ++i)
    {   new.data[i] = duplicate_var (l.data[i]);
    }

    return new;
}

/* duplicate_fn: duplicate f */
_Function duplicate_fn (_Function f)
{
    _Function new;
    new.type = f.type;

    if (new.type == FN_BUILTIN)
    {   new.builtin = f.builtin;
    }
    else
    {
        new.fn = (LISPFunction){ .env =duplicate_env (f.fn.env ),
                                 .body=duplicate_var (f.fn.body) };
    }
    return new;
}

