/*
 * Environment
 *
 */

#include "environment.h"
#include "builtins.h"
#include "data.h"

#include <stdlib.h>



/* id_index: return the index of id */
size_t id_index (Environment const *e, Identifier id)
{
    for (size_t i = 0; i < e->len; ++i)
    {   if (stringcmp (id, e->names[i]) == 0)
        {   return i;
        }
    }
    return -1;
}


/* id_lookup: return the value associated with an identifier */
Var id_lookup (Environment const *e, Identifier id)
{
    size_t i = id_index (e, id);

    if (i != (size_t)-1)
    {   return e->values[i];
    }
    else if (e->parent != NULL)
    {   return id_lookup (e->parent, id);
    }
    else
    {   return mkerr_var (EC_UNBOUND_VAR, "");
    }
}

/* add_id: add an id to the environment
 * (eg for defining a new variable) */
void add_id (Environment *e, Identifier id, Var v)
{
    size_t i = id_index (e, id);

    if (i == (size_t)-1)
    {   e->names  = realloc (e->names , sizeof(*e->names ) * (e->len+1));
        e->values = realloc (e->values, sizeof(*e->values) * (e->len+1));

        e->names [e->len] = stringdup (id);
        e->values[e->len] = v;

        e->len++;
    }
    else
    {   e->values[i] = v;
    }
}

/* change_value: change the value associated with id
 * (eg for variable assignment) */
void change_value (Environment *e, Identifier id, Var new)
{
    size_t i = id_index (e, id);

    if (i != (size_t)-1)
    {   e->values[i] = new;
    }
    else if (e->parent != NULL)
    {   /* id not found in this env, check parent env */
        change_value (e->parent, id, new);
    }
    else
    {   /* TODO: variable not found error */
        error ("Variable '%s' not found", id.chars);
    }
}



char const *const builtin_names[] =
 {  "+",
    "-",
    "*",
    "/",
    "define",
    "lambda",
 };

Var (*builtin_funcptrs[])(List, Environment *) =
 {  _builtin_add,
    _builtin_sub,
    _builtin_mul,
    _builtin_div,
    _builtin_define,
    _builtin_lambda,
 };

#define LEN(arr)    (sizeof(arr)/sizeof(*arr))

/* get_default_environment: get an environment initialized
 *                          with builtins and global vars */
Environment *get_default_environment(void)
{
    Environment *env = calloc (1, sizeof(*env));

    env->parent = NULL;
    env->len    = LEN(builtin_names);

    env->names  = calloc (env->len, sizeof(*env->names ));
    env->values = calloc (env->len, sizeof(*env->values));

    for (size_t i = 0; i < env->len; ++i)
    {
        BuiltIn   _bltin = { .fn=builtin_funcptrs[i] };
        _Function _func  = { .type   =FN_BUILTIN,
                             .builtin=_bltin
                           };

        env->values[i] = (Var){ .type=VAR_FUNCTION,
                                .fn  =_func
                              };
        env->names[i] = mkstring (builtin_names[i]);
    }
    return env;
}

/* free_env: free memory used by an Environment */
void free_env (Environment *e)
{
    for (size_t i = 0; i < e->len; ++i)
    {   free_string (e->names [i]);
        free_var    (e->values[i]);
    }
    free (e->names );
    free (e->values);
    free (e);
}

