/*
 * Variables
 *
 */

#include "data.h"
#include "var.h"

#include <stdlib.h>
#include <string.h>



Var *var_atom (Atom a)
{
    Var *new = new_var (VAR_ATOM);
    new->a = a;
    return new;
}

Var *var_pair (Var *car, Var *cdr)
{
    Var *new = new_var (VAR_PAIR);
    new->car = car;
    new->cdr = cdr;
    return new;
}

Var *var_nil(void)
{   return new_var (VAR_NIL);
}
Var *var_undefined(void)
{   return new_var (VAR_UNDEFINED);
}


Var *var_true(void)
{   return var_atom (atm_boolean (true));
}

Var *var_false(void)
{   return var_atom (atm_boolean (false));
}


Atom atm_boolean (bool b)
{   Atom a;
    a.boolean = b;
    return a;
}

Atom atm_number (double n)
{   Atom a;
    a.num = n;
    return a;
}

Atom atm_str (String s)
{   Atom a;
    a.str = s;
    return a;
}

Atom atm_sym (String s)
{   Atom a;
    a.sym = s;
    return a;
}

Atom atm_id (Identifier i)
{   Atom a;
    a.id = i;
    return a;
}

Atom atm_err (Error e)
{   Atom a;
    a.err = e;
    return a;
}

Atom atm_fn (_Function f)
{   Atom a;
    a.fn = f;
    return a;
}



/* new_var: create a new Var */
Var *new_var (enum VarType t)
{
    Var *ref  = GC_malloc (sizeof(*ref));
    ref->type = t;

    return ref;
}



/* The Elementary S-functions and Predicates  */
Var *atom (Var *x)
{   if (x->type == VAR_ATOM)
    {   return var_true();
    }
    else
    {   return var_false();
    }
}

Var *eq (Var *x, Var *y)
{
    if (atom (x)->a.boolean  && atom (y)->a.boolean)
    {
        if (memcmp (x, y, sizeof(*x)) == 0)
        {   return var_true();
        }
        else
        {   return var_false();
        }
    }
    else
    {   return var_undefined();
    }

}

Var *car (Var *x)
{
    if (x->type == VAR_PAIR)
    {   return x->p.car;
    }
    else
    {   return var_undefined();
    }
}

Var *cdr (Var *x)
{
    if (x->type == VAR_PAIR)
    {   return x->p.cdr;
    }
    else
    {   return var_undefined();
    }
}

Var *cons (Var *x, Var *y)
{   return var_pair (x, y);
}

