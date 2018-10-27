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
Var *id_lookup (Environment const *e, Identifier id)
{
    size_t i = id_index (e, id);

    if (i != (size_t)-1)
    {   return e->values[i];
    }
    else if (e->parent != NULL)
    {   return id_lookup (e->parent, id);
    }

    return mkerr_var (EC_UNBOUND_VAR, "'%s'", id.chars);
}

/* add_id: add an id to the environment
 * (eg for defining a new variable) */
void add_id (Environment *e, Identifier id, Var const *v)
{
    size_t i = id_index (e, id);

    if (i == (size_t)-1)
    {   e->names  = GC_realloc (e->names , sizeof(*e->names )
                                * (e->len+1));
        e->values = GC_realloc (e->values, sizeof(*e->values)
                                * (e->len+1));

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
void change_value (Environment *e, Identifier id, Var const *new)
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
    {   error ("%s -- Var '%s' not found", __func__, id.chars);
    }
}



struct
{   char const *const name;
    Var *(*funcptr)(List, Environment *);
} builtins[] =
 {  { "+",          _builtin_add        },
    { "-",          _builtin_sub        },
    { "*",          _builtin_mul        },
    { "/",          _builtin_div        },
    { "define",     _builtin_define     },
    { "lambda",     _builtin_lambda     },
    { "if",         _builtin_if         },
    { "set!",       _builtin_set        },
    { "include",    _builtin_include    },
    { "include-ci", _builtin_include_ci },

 };

#define LEN(arr)    (sizeof(arr)/sizeof(*arr))

/* get_default_environment: get an environment initialized
 *                          with builtins and global vars */
Environment *get_default_environment(void)
{
    Environment *env = GC_malloc (sizeof(*env));

    env->parent = NULL;
    env->len    = LEN(builtins);

    env->names  = GC_malloc (env->len * sizeof(*env->names ));
    env->values = GC_malloc (env->len * sizeof(*env->values));

    for (size_t i = 0; i < env->len; ++i)
    {
        BuiltIn   _bltin = { .fn  =builtins[i].funcptr,
                             .name=builtins[i].name     };
        _Function _func  = { .type   =FN_BUILTIN,
                             .builtin=_bltin
                           };

        env->values[i]     = new_var (VAR_FUNCTION);
        env->values[i]->fn = _func;

        env->names[i] = mkstring (builtins[i].name);
    }
    return env;
}

/* duplicate_env: duplicate e */
Environment *duplicate_env (Environment *e)
{
    Environment *new = GC_malloc (sizeof(*new));

    new->len    = e->len;
    new->names  = GC_malloc (new->len * sizeof(*new->names));
    new->values = GC_malloc (new->len * sizeof(*new->values));


    for (size_t i = 0; i < e->len; ++i)
    {
        new->names [i] = stringdup     (e->names [i]);
        new->values[i] = duplicate_var (e->values[i]);
    }

    if (e->parent != NULL)
    {
        new->parent = duplicate_env (e->parent);
    }

    return new;
}

