/*
 * LISP Builtin functions
 *
 * ie + - * /, etc.
 *
 *
 * NOTE: MKBUILTIN returns `Var *_builtin_[argument] (size_t argc, Var **argv, Environment env)
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
    Var *result    = new_var(VAR_NUMBER);
    result->number = 0;

    if (argv.len > 0)
    {
        for (size_t i = 0; i < argv.len; ++i)
        {
            Var *n = eval (argv.data[i], env);
            if (n->type == VAR_NUMBER)
            {   result->number += n->number;
            }
            else
            {   return NOT_A_NUMBER_ERROR(argv.data[i]);
            }
        }
        return result;
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(1, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}

/* sub: Subtraction. Can take infinite arguments
 *  NOTE: Also supports negation, ie (- 1) ===> -1  */
MKBUILTIN(sub)
{
    Var *result    = new_var (VAR_NUMBER);
    result->number = argv.data[0]->number;

    if (argv.len > 0)
    {   if (argv.data[0]->type != VAR_NUMBER)
        {   return NOT_A_NUMBER_ERROR(argv.data[0]);
        }

        /* more than 1 arg means subtraction */
        if (argv.len > 1)
        {
            for (size_t i = 1; i < argv.len; ++i)
            {
                Var *n = eval (argv.data[i], env);
                if (n->type == VAR_NUMBER)
                {   result->number -= n->number;
                }
                else
                {   return NOT_A_NUMBER_ERROR(argv.data[i]);
                }
            }
            return result;
        }
        /* 1 arg means negation */
        else
        {   result->number = -1 * argv.data[0]->number;
            return result;
        }
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(1, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}

/* mul: Multiplication. Can take infinite arguments */
MKBUILTIN(mul)
{
    Var *result    = new_var (VAR_NUMBER);
    result->number = 1;

    if (argv.len > 0)
    {
        for (size_t i = 0; i < argv.len; ++i)
        {
            Var *n = eval (argv.data[i], env);
            if (n->type == VAR_NUMBER)
            {   result->number *= n->number;
            }
            else
            {   return NOT_A_NUMBER_ERROR(argv.data[i]);
            }
        }
        return result;
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(1, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}

/* div: Division. Can take infinite arguments */
MKBUILTIN(div)
{
    Var *result    = new_var (VAR_NUMBER);
    result->number = 1;

    if (argv.len > 0)
    {
        Var *n = eval (argv.data[0], env);
        if (n->type == VAR_NUMBER)
        {   result->number = n->number;
        }
        else
        {   return NOT_A_NUMBER_ERROR(argv.data[0]);
        }

        for (size_t i = 1; i < argv.len; ++i)
        {
            n = eval (argv.data[i], env);
            if (n->type == VAR_NUMBER)
            {   result->number /= n->number;
            }
            else
            {   return NOT_A_NUMBER_ERROR(argv.data[i]);
            }
        }
        return result;
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(1, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}


/* define: takes two arguments -- a Symbol and a Var.
 *         adds the Var * to the current Environment
 *         under the Symbol's name */
MKBUILTIN(define)
{
    if (argv.len == 2)
    {
        if (argv.data[0]->type == VAR_IDENTIFIER)
        {   add_id (env,
                    argv.data[0]->id,
                    eval (argv.data[1], env));
            return UNDEFINED;
        }
        else
        {   return mkerr_var (EC_INVALID_ARG,
                              "%v is not an identifier",
                              argv.data[0]);
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
        /* formals = arguments */
        Var *formals = argv.data[0],
            *body    = argv.data[1];

        if (formals->type != VAR_LIST)
        {   return mkerr_var (EC_INVALID_ARG,
                              "%s -- '%v' is not a list",
                              __func__, formals);
        }

        debug ("lambda got arglist '%v' (length %zi)",
               formals, formals->list.len);


        /* Create a new Environment, filling it's `names' list
         * with the ids of the Identifiers in `formals' */
        Environment *fn_env = GC_malloc (sizeof(*fn_env));
        fn_env->len    = 0;
        fn_env->parent = env;

        for (size_t i = 0; i < formals->list.len; ++i)
        {   add_id (fn_env,
                    formals->list.data[i]->id,
                    UNDEFINED);
            debug ("NAME %zi = '%s'",
                   i,
                   fn_env->names[i].chars);
        }

        /* Return the Function as a Var */
        LISPFunction f = { .body=body,
                           .env =fn_env     };
        _Function func = { .type=FN_LISPFN,
                           .fn  =f          };

        Var *result = new_var (VAR_FUNCTION);
        result->fn  = func;

        return result;
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(2, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}



#undef MKBUILTIN
#undef _GETBUILTINNAME

#undef NOT_A_NUMBER_ERROR
#undef ARG_COUNT_MISMATCH_ERROR
#undef GENERAL_FAILURE_ERROR

