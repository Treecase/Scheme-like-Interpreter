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

#define NOT_A_NUMBER_ERROR(arg)                 (mkerr_var (EC_INVALID_ARG, "%s -- Invalid type. %v is not a Number", __func__, arg))
#define ARG_COUNT_MISMATCH_ERROR(requires, got) (mkerr_var (EC_INVALID_ARG,\
                                                            ((got) > (requires))? "%s -- Too many arguments. Max %zi, got %zi"\
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
                              "%v is not an Identifier",
                              argv.data[0]);
        }
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(2, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}


/* lambda:  (lambda <formals> <body>)
 *  where <formals> can be of the form:
 *   (<variable 1> ... <variable N>),
 *   <variable>,
 *   or (<variable 1> ... <variable N> . <extra>)
 *
 * In the first case, the procedure takes a fixed number (N) of arguments,
 * where the Nth argument is passed in the Nth variable.
 *
 * In the second case, the procedure can take any number of arguments,
 * which are passed as a list stored in <variable>.
 *
 * In the final case, the procedure takes N or more arguments, where the
 * first N arguments are passed in the <variable>s, and the remaining
 * arguments are passed as a list stored in <extra>.
 *
 *
 * In any case, `lambda' returns a Function object, which, when evaluated,
 * will eval each expression in <body>. `Lambda' will also generate a new
 * associated Environment object which contains a reference to the
 * Environment that `lambda' was called in. */

/* TODO: support list-binding and dotted-tail style formals (pg 13) */
/* TODO: error if a variable name is repeated in the formals list (pg 13) */
/* TODO: proper closures */
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
        fn_env->parent = duplicate_env (env);

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


/* if:  (if <test> <consequent> <alternate>)
 *      (if <test> <consequent>)
 *
 * Takes 2 or 3 arguments. The first is the test, which will
 * decide which branch to take. The second is the consequent,
 * which will be followed if <test> evaluates to `#f'. The
 * optional third argument is the alternate, which will be
 * followed if <test> DOES NOT evaluate to `#f' */
MKBUILTIN(if)
{
    Var *test       = argv.data[0],
        *consequent = argv.data[1],
        *alternate  = (argv.len == 3)? argv.data[2] : NULL;

    if (argv.len == 2 || argv.len == 3)
    {
        Var *test_result = eval (test, env);

        debug ("test: %v  consequent: %v  alternate: %v",
               test, consequent, alternate);

        /* The only False value in Scheme is #f, everything else is True */
        if (test_result->type == VAR_BOOLEAN\
         && test_result->boolean == false)
        {
            if (alternate != NULL)
            {   return eval (alternate, env);
            }
            else
            {   return UNDEFINED;
            }
        }
        else
        {   return eval (consequent, env);
        }
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR((argv.len > 3)? 3 : 2, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}


/* set!:    (set! <variable> <expression>)
 *
 * Sets the Var identified by <variable> to the evaluation of
 * <expression>. */
MKBUILTIN(set)
{
    if (argv.len == 2)
    {
        if (argv.data[0]->type == VAR_IDENTIFIER)
        {
            change_value (env,
                          argv.data[0]->id,
                          eval (argv.data[1], env));
            return UNDEFINED;
        }
        else
        {   return mkerr_var (EC_INVALID_ARG,
                              "%v is not an Identifier",
                              argv.data[0]);
        }
    }
    else
    {   return ARG_COUNT_MISMATCH_ERROR(2, argv.len);
    }
    return GENERAL_FAILURE_ERROR();
}


/* include:     (include    <string1> <string2> ...)
 * include-ci:  (include-ci <string1> <string2> ...)
 * (pg 14)
 */
#warning need to implement 'include-ci'
MKBUILTIN(include)
{
    argv = argv; env = env;
    return mkerr_var (EC_GENERAL, "%s not implemented", __func__);
}

MKBUILTIN(include_ci)
{
    argv = argv; env = env;
    return mkerr_var (EC_GENERAL, "%s not implemented", __func__);
}



#undef MKBUILTIN
#undef _GETBUILTINNAME

#undef NOT_A_NUMBER_ERROR
#undef ARG_COUNT_MISMATCH_ERROR
#undef GENERAL_FAILURE_ERROR

