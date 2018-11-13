/*
 * Evaluate token lists
 *
 */
/* TODO: tail-recursion (pg 11) */

#include "eval.h"
#include "data.h"
#include "global-state.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Var *call (_Function fn, List args, Environment *hostenv);



///* eval: evaluates a Symbol/List.
// *       for example, given "12.3", returns the Number `12.3',
// *       given the List ('+ '1 '2), returns the List (+ 1 2) */
//Var *eval (Var *dat, Environment *env)
//{
//    /* A list means a function application,
//     * so we pass it to call */
//    if (dat->type == VAR_LIST)
//    {   debug ("list '%v'\ngetting func...", dat);
//        Var *func = eval (dat->list.data[0], env);
//
//        if (func->type == VAR_FUNCTION)
//        {   debug ("applying '%v'...", func);
//
//            List args;
//            args.len =dat->list.len -1;
//            args.data=dat->list.data+1;
//
//            return call (func->fn, args, env);
//        }
//        else
//        {   return mkerr_var (EC_BAD_SYNTAX,
//                              "Cannot apply '%v'",
//                              func);
//        }
//    }
//    /* Identifier */
//    else if (dat->type == VAR_IDENTIFIER)
//    {
//        return id_lookup (env, dat->id);
//    }
//    return dat;
//}

///* call: evaluate a function */
//Var *call (_Function fn,
//          List args,
//          Environment *hostenv)
//{
//    /* When a LISPFunction (created with `lambda') is called, we
//     * must pass the arguments into the function as variables in
//     * its environment. To do this, we step through the argument
//     * list, and assign the values of each argument to each
//     * function local variable. Once this is done, we simply
//     * eval() the function body and return the result.
//     */
//
//    /* When a BuiltIn is called, the procedure is essentially the
//     * same as when a LISPFunction is called, except that BuiltIns
//     * can take a potentially infinite number of arguments, or may
//     * need to treat the arguments as Symbols, etc. The way this is
//     * handled is by passing the arguments as a List, and allowing
//     * the BuiltIn to interpret that as it will.
//     */
//
//    /* user-defined functions */
//    if (fn.type == FN_LISPFN)
//    {
//        LISPFunction func = fn.fn;
//
//        if (args.len != func.env->len)
//        {   return mkerr_var (EC_INVALID_ARG,
//                              "%s arguments. Expected %zi, got %zi",
//                              (args.len > func.env->len)? "Too many"
//                              : "Not enough",
//                              func.env->len,
//                              args.len);
//        }
//
//        /* add the passed args to the function's Environment */
//        for (size_t i = 0; i < args.len; ++i)
//        {   change_value (func.env,
//                          func.env->names[i],
//                          eval (args.data[i], hostenv));
//        }
//        func.env->parent = hostenv;
//
//        local_env = func.env;
//
//        /* call the function */
//        debug ("calling function");
//
//        Var *rval = eval (func.body, func.env);
//
//        debug ("returned '%v'", rval);
//        local_env = hostenv;
//        return rval;
//    }
//    /* builtin functions */
//    else if (fn.type == FN_BUILTIN)
//    {
//        debug ("got %zi args", args.len);
//
//        Var *result;
//        if (fn.builtin.fn)
//        {   result = fn.builtin.fn (args, hostenv);
//        }
//        else
//        {   result = mkerr_var (EC_GENERAL,
//                                "Operation Not Implemented");
//        }
//        return result;
//    }
//    return mkerr_var (EC_GENERAL, "in `%s' -- ???", __func__);
//}


Var *appq (Var *m);
/* apply: given `f' and `args', return the value of applying S-Function `f' to `args'  */
Var *apply (Var *f, Var *args)
{
    return eval (cons (f, appq (args)), NULL);
}

Var *list (Var *x, Var *y);
/* appq: recursively quote a list */
Var *appq (Var *m)
{
    if (m->type == VAR_NIL)
    {   return new_var (VAR_NIL);
    }
    else
    {   return cons (list (var_atom (atm_str (mkstring ("quote"))),
                           car (m)),
                     appq (cdr (m)));
    }
}

Var *assoc (Var *x, Var *y);
Var *evcon (Var *e, Var *a);
Var *evlis (Var *e, Var *a);
Var *append (Var *x, Var *y);
Var *pair (Var *x, Var *y);
/* eval: evaluate an expression */
Var *eval (Var *e, Var *a)
{
    if (e->type == VAR_ATOM)
    {
        return assoc (e, a);
    }
    else if (car (e)->type == VAR_ATOM)
    {
        char *s = car (e)->a.sym.chars;

        if (strcmp (s, "quote") == 0)
        {   return car (cdr (e));
        }
        else if (strcmp (s, "atom") == 0)
        {
            if (eval (car (cdr (e)), a)->type == VAR_ATOM)
            {   return var_true();
            }
            else
            {   return var_false();
            }
        }
        else if (strcmp (s, "eq") == 0)
        {
            if ( eval (car (cdr (e)), a)
              == eval (car (cdr (cdr (e))), a) )
            {
                return var_true();
            }
            else
            {   return var_false();
            }
        }
        else if (strcmp (s, "cond") == 0)
        {   return evcon (cdr (e), a);
        }
        else if (strcmp (s, "car") == 0)
        {   return car (eval (car (cdr (e)), a));
        }
        else if (strcmp (s, "cdr") == 0)
        {   return cdr (eval (car (cdr (e)), a));
        }
        else if (strcmp (s, "cons") == 0)
        {   return cons (eval (car (cdr (e)), a),
                         eval (car (cdr (cdr (e))), a));
        }
        else
        {
            return eval (cons (assoc (car (e), a),
                               evlis (cdr (e), a)),
                         a);
        }
    }
    else if (strcmp (car (car (e))->a.sym.chars, "label") == 0)
    {
        return eval (cons (car (cdr (cdr (car (e)))),
                           cdr (e)),
                     cons (list (car (cdr (car (e))),
                                 car (e)),
                           a));
    }
    else if (strcmp (car (car (e))->a.sym.chars, "lambda") == 0)
    {
        return eval (car (cdr (cdr (car (e)))),
                     append (pair (car (cdr (car (e))),
                                   evlis (cdr (e),
                                          a)),
                             a));
    }
    return var_undefined();
}

/* assoc: when `y' is a list of form ((u1 v1) (u2 v2) ... (uN vN)),
 *        and x is one of the `u's, return the corresponding v */
Var *assoc (Var *x, Var *y)
{
    if (eq (car (car (y)), x))
    {   return car (cdr (car (y)));
    }
    else
    {   return assoc (x, cdr (y));
    }
}

/* evcon: conditionally execute an expression */
Var *evcon (Var *c, Var *a)
{
    if (eval (car (car (c)), a)->a.boolean)
    {   return eval (car (cdr (car (c))), a);
    }
    else
    {   return evcon (cdr (c), a);
    }
}

/* evlist:  */
Var *evlis (Var *m, Var *a)
{
    if (m->type == VAR_NIL)
    {   return var_nil();
    }
    else
    {   return cons (eval (car (m), a), evlis (cdr (m), a));
    }
}

/* append: append x to y */
Var *append (Var *x, Var *y)
{
    if (x->type == VAR_NIL)
    {   return y;
    }
    else
    {
        return cons (car (x), append (cdr (x), y));
    }
}

/* pair: this function gives the list of pairs of corresponding
 *       elements of the lists x and y. */
Var *pair (Var *x, Var *y)
{
    if (x->type == VAR_NIL && y->type == VAR_NIL)
    {   return var_nil();
    }
    else if (x->type != VAR_ATOM && y->type != VAR_ATOM)
    {
        return cons (list (car (x), car (y)),
                     pair (cdr (x), cdr (y)));
    }
    return var_undefined();
}

/* list: given `e1', `e2', etc., return (e1 (e2 ... (eN . nil)...)) */
/* TODO: support infinite args? */
Var *list (Var *x, Var *y)
{
    return var_pair (x, var_pair (y, var_nil()));
}

