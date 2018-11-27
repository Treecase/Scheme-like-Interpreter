/*
 * Evaluate token lists
 *
 */
/* TODO: error checking */

#include "eval.h"
#include "data.h"
#include "global-state.h"
#include "builtins.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Var *pair_up (Var *a, Var *b);

Var *appq (Var *m);
Var *list (Var *x, Var *y);
Var *assoc (Var *x, Var *y);
Var *evcon (Var *e, Var *a);
Var *evlis (Var *e, Var *a);
Var *append (Var *x, Var *y);
Var *pair (Var *x, Var *y);



/* apply: given `f' and `args', return the value of applying
 *        S-Function `f' to `args' */
Var *apply (Var *f, Var *args, Var *env)
{
    if (f->a.fn.type == FN_BUILTIN)
    {   /* builtin */
        debug ("applying %v(%v) with %v", f, args, env);
        return f->a.fn.builtin.fn (args, env);
    }
    else
    {   /* lispfn */
        Var *p = pair_up (f->a.fn.fn.env, args);
        return eval (f->a.fn.fn.body, cons (car (p), env));
    }
}

/* pair_up: pair each `x' in a with each `y' in b
 *          (eg.  pair_up ((a b c) (1 2 3))
 *                ==> ((a 1) (b 2) (c 3))
 */
Var *pair_up (Var *a, Var *b)
{
    if (a->type == VAR_NIL && b->type == VAR_NIL)
    {   return var_nil();
    }
    else
    {   return cons (cons (car (a), car (b)),
                     pair_up (cdr (a), cdr (b)));
    }
}

/* appq: recursively quote a list */
Var *appq (Var *m)
{
    if (m->type == VAR_NIL)
    {   return var_nil();
    }
    else
    {   return cons (list (var_atom (atm_str (mkstring ("quote"))),
                           car (m)),
                     appq (cdr (m)));
    }
}

/* eval: evaluate an expression */
Var *eval (Var *e, Var *a)
{
    if (e->type == VAR_ATOM)
    {   /* atom */
        debug ("atom");
        return assoc (e, a);
    }
    else if (e->type == VAR_PAIR)
    {
        Var *first = eval (car (e), a);
        debug ("first ==> %v", first);

        if (first->type == VAR_ATOM && first->a.type == ATM_FUNCTION)
        {   /* function application */
            return apply (first, cdr (e), a);
        }
        else
        {   return mkerr_var (EC_BAD_SYNTAX, "cannot apply '%v'", first);
        }
    }
    else
    {   /* nil or undefined */
        debug ("nil or undefined");
        return e;
    }

    return mkerr_var (EC_BAD_SYNTAX, "eval -- %v does not match any evaluation rules", e);
}

/* assoc: when `y' is a list of form ((u1 v1) (u2 v2) ... (uN vN)),
 *        and x is one of the `u's, return the corresponding v */
Var *assoc (Var *x, Var *y)
{
    //debug ("assoc %v in %v", x, y);
    if (x->a.type == ATM_IDENTIFIER)
    {
        if (y->type == VAR_NIL)
        {   return mkerr_var (EC_UNBOUND_VAR, "%v not found", x);
        }
        else
        {
            if (eq (car (car (y)), x)->a.boolean)
            {   debug ("found %v ==> %v", x, cdr (car (y)));
                return cdr (car (y));
            }
            else
            {   return assoc (x, cdr (y));
            }
        }
    }
    else
    {   return x;
    }
}

/* evcon: conditionally execute an expression */
Var *evcon (Var *c, Var *a)
{
    if (c->type != VAR_NIL)
    {
        if (eval (car (car (c)), a)->a.boolean)
        {   return eval (car (cdr (car (c))), a);
        }
        else
        {   return evcon (cdr (c), a);
        }
    }
    return mkerr_var (EC_INVALID_ARG, "evcon -- got NULL conditional!");
}

/* evlis: evaluate each item in a list, returning
 *        a list of the results */
Var *evlis (Var *m, Var *a)
{
    if (m->type == VAR_NIL)
    {   return var_nil();
    }
    else
    {   return cons (eval (car (m), a),
                     evlis (cdr (m), a));
    }
}

/* append: append x to y */
Var *append (Var *x, Var *y)
{
    if (x->type == VAR_NIL)
    {   return y;
    }
    else
    {   return cons (car (x), append (cdr (x), y));
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
    return mkerr_var (EC_INVALID_ARG, "pair -- cannot cons %v and %v", x, y);
}

/* list: given `e1', `e2', etc., return (e1 (e2 ... (eN . nil)...)) */
Var *list (Var *x, Var *y)
{
    return var_pair (x, var_pair (y, var_nil()));
}

