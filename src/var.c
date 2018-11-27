/*
 * Variables
 *
 */

#include "data.h"
#include "var.h"

#include <stdlib.h>
#include <string.h>


int atomcmp (Atom a, Atom b);



Var *var_atom (Atom a)
{
    Var *new = new_var (VAR_ATOM);
    new->a = a;
    return new;
}

Var *var_pair (Var *car, Var *cdr)
{
    Var *new = new_var (VAR_PAIR);
    new->p.car = car;
    new->p.cdr = cdr;
    return new;
}

Var *var_nil(void)
{   return new_var (VAR_NIL);
}
Var *var_undefined(void)
{   return new_var (VAR_UNDEFINED);
}


Var *var_true(void)
{   return var_atom (atm_bool (true));
}

Var *var_false(void)
{   return var_atom (atm_bool (false));
}


Atom atm_bool (bool b)
{   Atom a;
    a.type = ATM_BOOLEAN;
    a.boolean = b;
    return a;
}

Atom atm_num (double n)
{   Atom a;
    a.type = ATM_NUMBER;
    a.num = n;
    return a;
}

Atom atm_str (String s)
{   Atom a;
    a.type = ATM_STRING;
    a.str = s;
    return a;
}

Atom atm_sym (String s)
{   Atom a;
    a.type = ATM_SYMBOL;
    a.sym = s;
    return a;
}

Atom atm_id (Identifier i)
{   Atom a;
    a.type = ATM_IDENTIFIER;
    a.id = i;
    return a;
}

Atom atm_err (Error e)
{   Atom a;
    a.type = ATM_ERROR;
    a.err = e;
    return a;
}

Atom atm_fn (_Function f)
{   Atom a;
    a.type = ATM_FUNCTION;
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

