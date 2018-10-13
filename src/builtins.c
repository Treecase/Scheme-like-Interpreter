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

#include <stdlib.h>



/* TODO: support string concatenation? */
MKBUILTIN(add)
{
    Var result = { .type=VAR_NUMBER,
                   .number=0
                 };

    if (argc > 0)
    {
        /* sum starts @ argv[0] */
        if (argv[0].type == VAR_NUMBER)
        {   result.number = argv[0].number;
        }
        else
        {   return mkerr_var (EC_INVALID_ARG,
                              "Invalid type. %v is not a number",
                              &argv[0]);
        }

        /* skip the first number, it's already added to the sum */
        for (size_t i = 1; i < argc; ++i)
        {
            if (argv[i].type == VAR_NUMBER)
            {   result.number += argv[i].number;
            }
            else
            {   return mkerr_var (EC_INVALID_ARG,
                                  "Invalid type: %v is not a number",
                                  &argv[i]);
            }
        }
        return result;
    }
    else
    {   return mkerr_var (EC_BAD_SYNTAX, "Not enough args");
    }
    return mkerr_var (EC_GENERAL,
                      "Something bad happened in `%s'",
                      __func__);
}

/* we can skimp out on this function because  A - B = A + (-B) */
MKBUILTIN(sub)
{
    if (argc > 0)
    {
        if (argc > 1)
        {
            Var *new_argv = calloc (argc, sizeof(*new_argv));

            /* first number remains positive */
            new_argv[0] = argv[0];

            /* invert all other arguments */
            for (size_t i = 1; i < argc; ++i)
            {
                new_argv[i] = argv[i];
                new_argv[i].number *= -1;
            }

            return _builtin_add (argc, new_argv, env);
        }
        else
        {   if (argv[0].type == VAR_NUMBER)
            {   return (Var){ .type=VAR_NUMBER,
                              .number=-argv[0].number
                            };
            }
            else
            {   return mkerr_var (EC_INVALID_ARG,
                                  "Invalid type: %v is not a number",
                                  &argv[0]);
            }
        }
    }
    else
    {   return mkerr_var (EC_INVALID_ARG, "Not enough args");
    }
    return mkerr_var (EC_GENERAL,
                      "Something bad happened in `%s'",
                      __func__);
}

MKBUILTIN(mul)
{
    Var result = { .type=VAR_NUMBER,
                   .number=0
                 };

    if (argc > 0)
    {
        if (argv[0].type == VAR_NUMBER)
        {   result.number = argv[0].number;
        }
        else
        {   return mkerr_var (EC_INVALID_ARG,
                              "Invalid type. %v is not a Number",
                              &argv[0]);
        }

        for (size_t i = 1; i < argc; ++i)
        {
            if (argv[i].type == VAR_NUMBER)
            {
                result.number *= argv[i].number;
            }
            else
            {   return mkerr_var (EC_INVALID_ARG,
                                  "Invalid type. %v is not a Number",
                                  &argv[i]);
            }
        }
        return result;
    }
    return mkerr_var (EC_GENERAL,
                      "Something bad happened in `%s'",
                      __func__);
}

/* we can skimp out on this one because  A / B = A * 1/B */
MKBUILTIN(div)
{
    if (argc > 0)
    {
        Var *new_argv = calloc (argc, sizeof(*new_argv));

        new_argv[0] = argv[0];
        for (size_t i = 1; i < argc; ++i)
        {
            new_argv[i] = argv[i];
            new_argv[i].number = 1 / new_argv[i].number;
        }
        return _builtin_mul (argc, new_argv, env);
    }
    else
    {   return mkerr_var (EC_INVALID_ARG, "Not enough args");
    }
    return mkerr_var (EC_GENERAL, "Something bad happened in `%s'",
                      __func__);
}


MKBUILTIN(define)
{
    if (argc == 2)
    {
        if (argv[0].type == VAR_SYMBOL)
        {   add_id (env, argv[0].sym, argv[1]);
            return argv[0];
        }
        else
        {   return mkerr_var (EC_INVALID_ARG,
                              "%v is not an identifier",
                              &argv[0]);
        }
    }
    else
    {   return mkerr_var (EC_INVALID_ARG,
                          "define takes exactly 2 arguments, got %zi",
                          argc);
    }
    return mkerr_var (EC_GENERAL,
                      "Something bad happened in `%s'",
                      __func__);
}



#undef MKBUILTIN
#undef _GETBUILTINNAME

