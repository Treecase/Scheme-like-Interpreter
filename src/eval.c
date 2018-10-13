/*
 * Evaluate token lists
 *
 */

#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Var call (_Function fn, Token const *const args, Environment *hostenv);



/* eval: get the value of a token */
Var eval (Token t, Environment *e)
{
    switch (t.type)
    {
    case TOK_NUMBER:
        return (Var){ .number=t.number,
                      .type=VAR_NUMBER
                    };
        break;

    case TOK_STRING:
        return (Var){ .str=t.str,
                      .type=VAR_STRING
                    };
        break;

    case TOK_IDENTIFIER:
      { Var val = id_lookup (e, t.id);
        if (val.type == VAR_ERROR && val.err.errcode == EC_UNBOUND_VAR)
        {   free_var (val);
            return (Var){ .type=VAR_SYMBOL,
                          .sym=stringdup (t.id)
                        };
        }
        else
        {   return val;
        }
      } break;

    case TOK_EXPR:
      {
        if (t.subexpr[0].type == TOK_IDENTIFIER)
        {
            Var fn = eval (t.subexpr[0], e);
            if (fn.type == VAR_FUNCTION)
            {   return call (fn.fn, t.subexpr+1, e);
            }
            else
            {   return mkerr_var (EC_BAD_SYNTAX, "1st list elem is not a function");
            }
        }
        else
        {   return mkerr_var (EC_BAD_SYNTAX, "1st list elem is not an id");
        }
#if 0
        List rlist = { .len=0,
                       .data=NULL
                     };
        for (size_t i = 0; t.subexpr[i].type != TOK_ENDEXPR; ++i)
        {
            rlist.data = realloc (rlist.data, sizeof(*rlist.data)*(i+1));
            rlist.data[i] = eval (t.subexpr[i], e);
            rlist.len++;
        }
        return (Var){ .list=rlist,
                      .type=VAR_LIST
                    };
#endif
      } break;

    case TOK_ENDEXPR:
        return mkerr_var (EC_BAD_SYNTAX, "how'd this get here?");
        break;
    }

    return mkerr_var (EC_BAD_SYNTAX, "Unknown token type");
}

/* call: evaluate a function */
Var call (_Function fn,
          Token const *const args,
          Environment *hostenv)
{
    /* When a LISPFunction is called, we must pass the arguments into
     * the function as variables in it's environment. To do this,
     * we step through the argument list, and assign the values of
     * each argument to each function variable a,b,c, etc. Once this
     * is done, we simply eval() the function body and return the
     * result.
     */

    /* When a Builtin is called, the procedure is essentially the
     * same as when a LISPFunction is called, except that builtins
     * can take infinite lists of arguments. The way this is handled
     * is by passing the arguments as a list, and allowing the
     * function to interpret that as it will.
     */

    /* user-defined functions */
    if (fn.type == FN_LISPFN)
    {
        LISPFunction func = fn.fn;

        /* pass the args */
        for (size_t i = 0; args[i].type != TOK_ENDEXPR; ++i)
        {
            Identifier var_name = func.env->names[i];

            /* set `var_name' to reference the value of arg[i] */
            change_value (func.env, var_name, eval (args[i], hostenv));
        }
        func.env->parent = hostenv;

        /* apply the function body to the args */
        List l = { .len=0,
                   .data=NULL
                 };
        for (size_t i = 0; func.body[i].type != TOK_ENDEXPR; ++i)
        {
            l.data = realloc (l.data, sizeof(*l.data)*(i+1));
            l.data[i] = eval (func.body[i], func.env);
            l.len++;
        }
        return (Var){ .list=l, .type=VAR_LIST };
    }
    /* builtin functions */
    else if (fn.type == FN_BUILTIN)
    {
        Var *argv = NULL;
        size_t argc = 0;
        for (; args[argc].type != TOK_ENDEXPR; ++argc)
            ;

        argv = calloc (argc, sizeof(*argv));
        for (size_t i = 0; i < argc; ++i)
        {   argv[i] = eval (args[i], hostenv);
        }

        Var result = { 0 };
        if (fn.builtin.fn)
        {   result = fn.builtin.fn (argc, argv, hostenv);
        }
        else
        {   result = mkerr_var (EC_BAD_SYNTAX,
                              "Operation Not Implemented");
        }
        free (argv);
        return result;
    }

    return mkerr_var (EC_BAD_SYNTAX, "in `call'");
}

