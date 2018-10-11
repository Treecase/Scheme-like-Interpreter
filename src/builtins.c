/*
 * LISP Builtin functions
 *
 * ie + - * /, etc.
 *
 */

#include "builtins.h"
#include "data.h"

#include <stdlib.h>



/* TODO: support string concatenation? */
Var _builtin_add (size_t argc, Var const *const argv)
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
        {   return mkerr_var (EC_INVALID_ARG, "Invalid type. %v is not a number", argv[0]);
        }

        /* skip the first number, it's already added to the sum */
        for (size_t i = 1; i < argc; ++i)
        {
            if (argv[i].type == VAR_NUMBER)
            {   result.number += argv[i].number;
            }
            else
            {   return mkerr_var (EC_INVALID_ARG, "Invalid type: %v is not a number", argv[i]);
            }
        }
        return result;
    }
    else
    {   return mkerr_var (EC_BAD_SYNTAX, "Not enough args");
    }

    return mkerr_var (EC_BAD_SYNTAX, "in add");
}

/* we can skimp out on this function because  A - B = A + (-B) */
Var _builtin_sub (size_t argc, Var const *const argv)
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

            return _builtin_add (argc, new_argv);
        }
        else
        {   if (argv[0].type == VAR_NUMBER)
            {   return (Var){ .type=VAR_NUMBER,
                              .number=-argv[0].number
                            };
            }
            else
            {   return mkerr_var (EC_INVALID_ARG, "Invalid type: %v is not a number", argv[0]);
            }
        }
    }
    else
    {   return mkerr_var (EC_INVALID_ARG, "Not enough args");
    }
}

Var _builtin_mul (size_t argc, Var const *const argv)
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
        {   return mkerr_var (EC_INVALID_ARG, "Invalid type. %v is not a Number", argv[0]);
        }

        for (size_t i = 1; i < argc; ++i)
        {
            if (argv[i].type == VAR_NUMBER)
            {
                result.number *= argv[i].number;
            }
            else
            {   return mkerr_var (EC_INVALID_ARG, "Invalid type. %v is not a Number", argv[i]);
            }
        }
        return result;
    }

    return mkerr_var (EC_BAD_SYNTAX, "in mul");
}

/* we can skimp out on this one because  A / B = A * 1/B */
Var _builtin_div (size_t argc, Var const *const argv)
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
        return _builtin_mul (argc, new_argv);
    }
    else
    {   return mkerr_var (EC_INVALID_ARG, "Not enough args");
    }
}

