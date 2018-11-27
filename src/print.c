/*
 * Printf extension for Vars, Tokens, etc.
 *
 */

#include "print.h"
#include "global-state.h"

#include <stdio.h>
#include <printf.h>



/* print_var: printf functionality for LISP Vars */
/* TODO: support formatting stuff like `%6.2v' */
int print_var (FILE *stream,
               struct printf_info const *info,
               void const *const *args)
{
    Var *v = *(Var **)(args[0]);

    if (v != NULL)
    {
        switch (v->type)
        {
        case VAR_ATOM:
            return fprintf (stream, "%T", &v->a);
            break;

        case VAR_PAIR:
            return fprintf (stream, "(%v . %v)", v->p.car, v->p.cdr);
            break;

        case VAR_NIL:
            return fprintf (stream, "<nil>");
            break;

        case VAR_UNDEFINED:
            return fprintf (stream, "<undefined>");
            break;
        }
        return fprintf (stream, "jasldjalssjd");
    }
    else
    {   return fprintf (stream, "NULL");
    }
    error ("Unknown Var type '%i'", v->type);
    return -1;
}

/* print_atom: printf functionality for Atoms */
/* TODO: support formatting stuff like `%6.2v' */
int print_atom (FILE *stream,
                struct printf_info const *info,
                void const *const *args)
{
    /* we get passed a Var pointer by printf */
    Atom *v = *(Atom **)(args[0]);

    if (v != NULL)
    {   switch (v->type)
        {
        case ATM_BOOLEAN:
            return fprintf (stream, "%s", v->boolean? "#t" : "#f");
            break;

        case ATM_NUMBER:
            return fprintf (stream, "%g", v->num);
            break;

        case ATM_STRING:
            return fprintf (stream, "\"%s\"", v->str.chars);
            break;

        case ATM_SYMBOL:
            return fprintf (stream, "'%s",
                            v->sym.chars);
            break;

        case ATM_IDENTIFIER:
            return fprintf (stream,
                            "|%s|",
                            v->id.chars);
            break;

        case ATM_ERROR:
            return fprintf (stream, "[%s: %s]",
                            err_msg (v->err),
                            v->err.flavour);
            break;

        case ATM_FUNCTION:
            if (v->fn.type == FN_LISPFN)
            {   return fprintf (stream, "<function %v>",
                                v->fn.fn.body);
            }
            else if (v->fn.type == FN_BUILTIN)
            {   return fprintf (stream, "<builtin %s at %p>",
                                v->fn.builtin.name,
                                v->fn.builtin.fn);
            }
            else
            {   return fprintf (stream, "unknown function type %i",
                                v->fn.type);
            }
            break;
        }
    }
    else
    {   return error ("%s got NULL", __func__);
    }
    error ("Unknown Atom type '%i'", v->type);
    return -1;
}

/* print_var_arginfo: used by printf to pass us
 *                    input in the correct form;
 *                    in this case, a pointer */
int print_var_arginfo (struct printf_info const *info,
                       size_t n,
                       int *argtypes)
{   if (n > 0)
    {   argtypes[0] = PA_POINTER;
    }
    return 1;
}


/* init_print: setup custom printf handlers */
void init_print(void)
{   register_printf_function ('v', print_var, print_var_arginfo);
    register_printf_function ('T', print_atom, print_var_arginfo);
}

