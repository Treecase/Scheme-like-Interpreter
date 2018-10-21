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
    /* we get passed a Var pointer by printf */
    Var *v = *(Var **)(args[0]);

    if (v == NULL)
    {   error ("print_var got NULL!");
        return -1;
    }

    switch (v->type)
    {
    case VAR_NUMBER:
        return fprintf (stream, "%g", v->number);
        break;

    case VAR_STRING:
        return fprintf (stream, "\"%s\"", v->str.chars);
        break;

    case VAR_FUNCTION:
        if (v->fn.type == FN_LISPFN)
        {   return fprintf (stream, "<function at %p>",
                            v->fn.fn.body.data);
        }
        else if (v->fn.type == FN_BUILTIN)
        {   return fprintf (stream, "<builtin at %p>",
                            v->fn.builtin.fn);
        }
        else
        {   return fprintf (stream, "unknown function type %i",
                            v->fn.type);
        }
        break;

    case VAR_LIST:
     {  int len = 2;
        fputc ('(', stream);
        for (size_t i = 0; i < v->list.len-1; ++i)
        {
            len += fprintf (stream, "%v", &v->list.data[i]) + 1;
            fputc (' ', stream);
        }
        if (v->list.len > 0)
        {   len += fprintf (stream, "%v",
                            &v->list.data[v->list.len-1]);
        }
        fputc (')', stream);
        return len;
     }  break;

    case VAR_ERROR:
        return fprintf (stream, "[%s: %s]",
                        err_msg (v->err),
                        v->err.flavour);
        break;

    case VAR_SYMBOL:
        return fprintf (stream, "%s",
                        v->sym.chars);
        break;

    case VAR_IDENTIFIER:
      { Var i = id_lookup (local_env, v->id);
        return fprintf (stream,
                        "%v",
                        &i);
      } break;

    case VAR_UNDEFINED:
        return fprintf (stream, "<undefined>");
        break;
    }
    error ("Unknown Var type '%i'", v->type);
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
}

