/*
 * Evaluate token lists
 *
 */

#include "eval.h"
#include "data.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Var call (_Function fn, List args, Environment *hostenv);



/* eval: evaluates a Symbol/List.
 *       for example, given "12.3", returns the Number `12.3',
 *       given the List ('+ '1 '2), returns the List (+ 1 2) */
Var eval (Var dat, Environment *env)
{
    /* if we're given a real Var, just return it */
    if (dat.type != VAR_SYMBOL && dat.type != VAR_LIST)
    {   debug ("redundant call to `eval'!");
        return dat;
    }

    /* A list means a function application,
     * so we pass it to call */
    if (dat.type == VAR_LIST)
    {   debug ("list '%v'\ngetting func...", &dat);
        Var func = eval (dat.list.data[0], env);
        if (func.type == VAR_FUNCTION)
        {   debug ("applying '%v'...", &func);
            List args  = { .len=dat.list.len-1,
                           .data=dat.list.data+1 };

            Var result = call (func.fn, args, env);

            return result;
        }
        else
        {   return mkerr_var (EC_BAD_SYNTAX,
                              "Cannot apply '%v'",
                              &func);
        }
    }
    else
    {
        Var out = { 0 };
        String sym = dat.sym;

        /* Quoted values begin with ' */
        if (sym.chars[0] == '\'')
        {   debug ("quoted value '%s'", sym.chars);
            out.type = VAR_SYMBOL;
            out.sym  = mkstring (sym.chars+1);
        }
        /* Strings start with ", so if the word begins
         * with ", we assume it's a String. */
        if (sym.chars[0] == '"')
        {   debug ("string '%s'", sym.chars);
            bool found_endquote = false;

            size_t i = 1;
            for (; i < sym.len; ++i)
            {   if (sym.chars[i] == '"')
                {   found_endquote = true;
                    i -= 1;
                    break;
                }
            }

            if (found_endquote)
            {   out.type      = VAR_STRING;
                out.str.chars = strndup (sym.chars+1, i);
                out.str.len   = i;
                out.str.size  = i+1;
            }
            else
            {   return mkerr_var (EC_BAD_SYNTAX,
                                  "Unbalanced quotes -- '%s'",
                                  sym.chars);
            }

        }
        /* Numbers start with a digit, a sign, or a decimal, so if
         * the word starts with [1-9], +, -, or ., we assume it's
         * a Number */
#define IS_NUMBER(ch)   (isdigit (ch)\
                        || ch == '+' || ch == '-'\
                        || ch == '.')
        else if (IS_NUMBER(sym.chars[0]))
#undef IS_NUMBER
        {   debug ("number '%s'", sym.chars);
            char *end;
            out.type   = VAR_NUMBER;
            out.number = strtod (sym.chars, &end);

            /* strtod stores a pointer to the last character used
             * in conversion in `end'. If the word is a valid Number,
             * the entire thing should be used by strtod. If strtod
             * did not use all the characters in the word, then the
             * word must be either an Identifier or an invalid
             * Number literal */
            if (end != sym.chars + sym.len)
            {   /* check if the symbol is an Identifer */
                Var val = id_lookup (env, sym);
                if (val.type == VAR_ERROR && val.err.errcode == EC_UNBOUND_VAR)
                {   return mkerr_var (EC_BAD_SYNTAX,
                                      "invalid Number literal '%s'",
                                      sym.chars);
                }
                else
                {   debug ("^^ wrong, ID");
                    return val;
                }
            }
        }
        /* Identifiers can be any sequence of characters, so if the
         * symbol isn't a Number, and it isn't a String, then it must
         * be a Symbol -- we have to look up it's value in the
         * Environment */
        else
        {   debug ("identifier '%s'", sym.chars);
            out = id_lookup (env, sym);
        }

        return out;
    }
    return mkerr_var (EC_GENERAL,
                      "failure in `%s' -- ???",
                      __func__);
}

/* call: evaluate a function */
Var call (_Function fn,
          List args,
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

        /* add the passed args to the function's Environment */
        for (size_t i = 0; i < args.len; ++i)
        {   change_value (func.env,
                          func.env->names[i],
                          eval (args.data[i], hostenv));
        }
        func.env->parent = hostenv;

        /* call the function */
        debug ("calling function");
        Var body = { .type=VAR_LIST, .list=func.body };
        Var rval = eval (body, func.env);

        debug ("returned '%v'", &rval);
        return rval;
    }
    /* builtin functions */
    else if (fn.type == FN_BUILTIN)
    {
        debug ("got %zi args", args.len);

        Var result;
        if (fn.builtin.fn)
        {   result = fn.builtin.fn (args, hostenv);
        }
        else
        {   result = mkerr_var (EC_GENERAL,
                                "Operation Not Implemented");
        }
        return result;
    }
    return mkerr_var (EC_GENERAL, "in `%s' -- ???", __func__);
}

