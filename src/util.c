/*
 * Utility functions for LISP
 *
 */

#include "util.h"



/* atom: return #t if x is an Atom, else #f */
Var *atom (Var *x)
{
    if (x->type == VAR_ATOM)
    {   return var_true();
    }
    return var_false();
}

/* eq: return #t if x == y, else #f */
Var *eq (Var *x, Var *y)
{
    if (x->type == y->type)
    {
        switch (x->type)
        {
        case VAR_ATOM:
            if (x->a.type == y->a.type)
            {
                switch (x->a.type)
                {
                case ATM_BOOLEAN:
                    if (x->a.boolean == y->a.boolean)
                    {   return var_true();
                    }
                    break;
                case ATM_NUMBER:
                    if (x->a.num == y->a.num)
                    {   return var_true();
                    }
                    break;

                case ATM_STRING:
                case ATM_SYMBOL:
                case ATM_IDENTIFIER:
                    if (stringcmp (x->a.str, y->a.str) == 0)
                    {   return var_true();
                    }
                    break;

                case ATM_ERROR:
                    if (x->a.err.errcode == y->a.err.errcode)
                    {   return var_true();
                    }
                    break;
                case ATM_FUNCTION:
                    if (x->a.fn.type == y->a.fn.type)
                    {
                        switch (x->a.fn.type)
                        {
                        case FN_LISPFN:
                            if (eq (x->a.fn.fn.body, y->a.fn.fn.body)->a.boolean && eq (x->a.fn.fn.env, y->a.fn.fn.env)->a.boolean)
                            {   return var_true();
                            }
                            break;
                        case FN_BUILTIN:
                            if (x->a.fn.builtin.fn == y->a.fn.builtin.fn)
                            {   return var_true();
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            break;

        case VAR_PAIR:
            if (eq (car (x), car (y))->a.boolean
             && eq (cdr (x), cdr (y))->a.boolean)
            {   return var_true();
            }
            break;

        /* these are constants */
        case VAR_NIL:
        case VAR_UNDEFINED:
            return var_true();
            break;
        }
    }
    return var_false();
}

/* car: return the first element of a Pair */
Var *car (Var *p)
{
    if (p->type == VAR_PAIR)
    {   return p->p.car;
    }
    else
    {   return mkerr_var (EC_INVALID_ARG, "%v is not a Pair", p);
    }
}

/* cdr: return the second element of a Pair */
Var *cdr (Var *p)
{
    if (p->type == VAR_PAIR)
    {   return p->p.cdr;
    }
    else
    {   return mkerr_var (EC_INVALID_ARG, "%v is not a Pair", p);
    }
}

/* car: construct a new Pair containing a and b */
Var *cons (Var *a, Var *b)
{
    return var_pair (a, b);
}



/* length_of_list: return the length of list */
int length_of_list (Var *list)
{
    int len = 0;
    for (Var *p = list; p->type == VAR_PAIR; p = cdr (p))
    {
        len++;
    }
    return len;
}

