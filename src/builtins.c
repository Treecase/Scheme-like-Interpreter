/*
 * LISP Builtin functions
 *
 * ie + - * /, etc.
 *
 *
 * NOTE: MKBUILTIN returns `Var _builtin_[argument] (size_t argc, Var *argv, Environment env)
 *
 */

#define _NO_UNDEF_MKDEFINE
#include "builtins.h"
#include "data.h"
#include "eval.h"

#include <stdlib.h>

#define NOT_A_NUMBER_ERROR(arg)                 (mkerr_var (EC_INVALID_ARG, "%s -- Invalid type. %v is not a number", __func__, arg))
#define ARG_COUNT_MISMATCH_ERROR(requires, got) (mkerr_var (EC_INVALID_ARG,\
                                                            (got > requires)? "%s -- Too many arguments. Max %zi, got %zi"\
                                                                            : "%s -- Too few arguments. Min %zi, got %zi",\
                                                            __func__,\
                                                            requires,\
                                                            got))
#define GENERAL_FAILURE_ERROR()                 (mkerr_var (EC_GENERAL, "%s -- ???", __func__))



/* add: Addition. Can take infinite arguments */
MKBUILTIN(add)
{
    Var result = { .type=VAR_NUMBER,
                   .number=0
                 };

    if (argv.len > 0)
    {
        for (size_t i = 0; i < argv.len; ++i)
        {
            Var n = eval (argv.data[i], env);
            if (n.type == VAR_NUMBER)
            {   result.number += n.number;
            }
            else
            {   return NOT_A_NUMBER_ERROR(&argv.data[i]);
            }
        }
        return result;
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(0, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}

/* sub: Subtraction. Can take infinite arguments
 *  NOTE: Also supports negation, ie (- 1) ===> -1  */
MKBUILTIN(sub)
{
    if (argv.len > 0)
    {   if (argv.data[0].type != VAR_NUMBER)
        {   return NOT_A_NUMBER_ERROR(&argv.data[0]);
        }

        /* more than 1 arg means subtraction */
        if (argv.len > 1)
        {
            Var result = { .type  =VAR_NUMBER,
                           .number=argv.data[0].number };

            for (size_t i = 1; i < argv.len; ++i)
            {
                Var n = eval (argv.data[i], env);
                if (n.type == VAR_NUMBER)
                {   result.number -= n.number;
                }
                else
                {   return NOT_A_NUMBER_ERROR(&argv.data[i]);
                }
            }
            return result;
        }
        /* 1 arg means negation */
        else
        {   return (Var){ .type  =VAR_NUMBER,
                          .number=(-1) * argv.data[0].number };
        }
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(0, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}

/* mul: Multiplication. Can take infinite arguments */
MKBUILTIN(mul)
{
    Var result = { .type=VAR_NUMBER,
                   .number=0
                 };

    if (argv.len > 0)
    {
        Var n = eval (argv.data[0], env);
        if (n.type == VAR_NUMBER)
        {   result.number = n.number;
        }
        else
        {   return NOT_A_NUMBER_ERROR(&argv.data[0]);
        }

        for (size_t i = 1; i < argv.len; ++i)
        {
            n = eval (argv.data[i], env);
            if (n.type == VAR_NUMBER)
            {   result.number *= n.number;
            }
            else
            {   return NOT_A_NUMBER_ERROR(&argv.data[i]);
            }
        }
        return result;
    }
    return GENERAL_FAILURE_ERROR();
}

/* div: Division. Can take infinite arguments */
MKBUILTIN(div)
{
    if (argv.len > 0)
    {
        Var result = { .type =VAR_NUMBER,
                       .number=0          };

        Var n = eval (argv.data[0], env);
        if (n.type == VAR_NUMBER)
        {   result.number = n.number;
        }
        else
        {   return NOT_A_NUMBER_ERROR(&argv.data[0]);
        }

        for (size_t i = 1; i < argv.len; ++i)
        {
            n = eval (argv.data[i], env);
            if (n.type == VAR_NUMBER)
            {   result.number /= n.number;
            }
            else
            {   return NOT_A_NUMBER_ERROR(&argv.data[i]);
            }
        }
        return result;
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(0, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}


/* define: takes two arguments -- a Symbol and a Var.
 *         adds the Var to the current Environment
 *         under the Symbol's name */
MKBUILTIN(define)
{
    if (argv.len == 2)
    {
        if (argv.data[0].type == VAR_SYMBOL)
        {   add_id (env, argv.data[0].sym, eval (argv.data[1], env));
            return id_lookup (env, argv.data[0].sym);
        }
        else
        {   return mkerr_var (EC_INVALID_ARG,
                              "%T is not an identifier",
                              &argv.data[0]);
        }
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(2, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}


/* lambda: takes 2 list arguments -- the arguments
 *         and the procedure body, and creates a
 *         new procedure from these.
 *         eg (lambda (a b) (* a b) */
MKBUILTIN(lambda)
{
    if (argv.len == 2)
    {
        if (argv.data[0].type != VAR_LIST)
        {   return mkerr_var (EC_INVALID_ARG,
                              "%s -- '%v' is not a list",
                              __func__, &argv.data[0]);
        }
        if (argv.data[1].type != VAR_LIST)
        {   return mkerr_var (EC_INVALID_ARG,
                              "%s -- '%v' is not a list",
                              __func__, &argv.data[1]);
        }

        debug ("lambda got arglist '%v' (length %zi)",
               &argv.data[0], argv.data[0].list.len);

        /* TODO: clean this section up */
        Environment *fn_env = calloc (1, sizeof(*fn_env));
        fn_env->len    = 0;
        fn_env->parent = env;


        for (size_t i = 0; i < argv.data[0].list.len; ++i)
        {   add_id (fn_env, argv.data[0].list.data[i].sym, (Var){ 0 });
            debug ("NAME %zi = '%s'",
                   i,
                   fn_env->names[i].chars);
        }

        List body;
        body.len  = argv.data[1].list.len;
        body.data = malloc (body.len * sizeof(*body.data));
        for (size_t i = 0; i < body.len; ++i)
        {   body.data[i] = vardup (argv.data[1].list.data[i]);
        }

        LISPFunction f = { .body=body,
                           .env =fn_env     };
        _Function func = { .type=FN_LISPFN,
                           .fn  =f          };

        Var result;
        result.type = VAR_FUNCTION;
        result.fn   = func;

        return result;
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(2, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}



#undef MKBUILTIN
#undef _GETBUILTINNAME

