/*
 * Evaluate token lists
 *
 */

#include "eval.h"
#include "data.h"
#include "builtins.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Var *pair_up (Var *a, Var *b);
Var *assoc (Var *id, Var *env);



/* apply: given an S-Function `f' and an Environment `args',
 *        return the value of applying `f' over `args' */
Var *apply (Var *f, Var *args, Var *env)
{
    if (f->type == VAR_ATOM && f->a.type == ATM_FUNCTION)
    {
        if (f->a.fn.type == FN_BUILTIN)
        {   /* builtin */
            debug ("applying %v(%v) with %v", f, args, env);
            return f->a.fn.builtin.fn (args, env);
        }
        else
        {   /* lispfn */
            Var *p = pair_up (f->a.fn.fn.env, args);
            Var *tmpenv = cons (car (p), env);

            for (Var *t = cdr (p); t->type != VAR_NIL; t = cdr (t))
            {   tmpenv = cons ( car (t), tmpenv );
            }
            debug ("applying %v[%v] with %v", f, args, tmpenv);
            return eval (f->a.fn.fn.body, cons (car (p), tmpenv));
        }
    }
    return mkerr_var (EC_GENERAL, "%v is not a Function", f);
}

/* pair_up: for `a' and `b' of form (x1 x2 ... xN) and (y1 y2 ... yN,
 *          return a list of form ((x1 y1) (x2 y2) ... (xN yN)) */
Var *pair_up (Var *a, Var *b)
{
    if (a->type == VAR_NIL && b->type == VAR_NIL)
    {   return var_nil();
    }
    else
    {   debug ("pairing %v  and  %v", car (a), car (b));
        return cons (cons (car (a), car (b)),
                     pair_up (cdr (a), cdr (b)));
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

/* assoc: `env' is a list of form ((id1 val1) (id2 val2) ... (idN valN));
 *         return the `val' corresponding to `id' */
Var *assoc (Var *id, Var *env)
{
    //debug ("assoc %v in %v", id, env);
    if (id->a.type == ATM_IDENTIFIER)
    {
        if (env->type == VAR_NIL)
        {   return mkerr_var (EC_UNBOUND_VAR, "%v not found", id);
        }
        else
        {
            if (eq (car (car (env)), id)->a.boolean)
            {   debug ("found %v ==> %v", id, cdr (car (env)));
                return cdr (car (env));
            }
            else
            {   return assoc (id, cdr (env));
            }
        }
    }
    else
    {   return id;
    }
}

