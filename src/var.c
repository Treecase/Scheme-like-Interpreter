/*
 * Variables
 *
 */

#include "data.h"
#include "var.h"
#include "token.h"

#include <stdlib.h>
#include <string.h>



/* new_var: create a new Var */
Var *new_var (enum VarType t)
{
    Var *ref  = GC_malloc (sizeof(*ref));
    ref->type = t;

    return ref;
}

