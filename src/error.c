/*
 * LISP Errors
 *
 */

#include "error.h"
#include "data.h"

#include <stdio.h>  /* vsnprintf */
#include <stdarg.h> /* va_list, etc. */



/* error messages associated with specific error codes */
char const *const errmsg[] =
{   [EC_UNBOUND_VAR] =  "Unbound Variable",
    [EC_BAD_SYNTAX]  =  "Syntax Error",
    [EC_INVALID_ARG] =  "Invalid Argument",
    [EC_GENERAL]     =  "Generic Error",
};



/* _mkerr_va: mkerr that accepts a va_list */
static Error _mkerr_va (Errcode t, char *flavour, va_list ap)
{
    int len = 0;
    char *buf = NULL;

    len = vsnprintf (NULL, 0, flavour, ap);
    if (len < 0)
    {   fatal ("vsnprintf failed!");
    }
    else
    {
        len += 1;
        buf = GC_malloc (len);
        vsnprintf (buf, len, flavour, ap);

        return (Error){ .errcode=t,
                        .flavour=buf
                      };
    }
}

/* errmsg: get the error message associated with an error code */
char const *err_msg (Error e)
{
    if (e.errcode > _EC_COUNT || e.errcode < 0)
    {   fatal ("Invalid error code %i", e.errcode);
    }
    else
    {   return errmsg[e.errcode];
        return NULL;
    }
}

/* mkerr: return a new error of type t */
Error mkerr (Errcode t, char *flavour, ...)
{   va_list ap;
    va_start (ap, flavour);
    Error e = _mkerr_va (t, flavour, ap);
    va_end (ap);
    return e;
}

/* mkerr_var: create an error Var with type t */
Var *mkerr_var (Errcode t, char *flavour, ...)
{
    va_list ap;
    va_start (ap, flavour);

    Var *r = new_var (VAR_ERROR);
    r->err = _mkerr_va (t, flavour, ap);

    va_end (ap);
    return r;
}

/* duplicate_err: duplicate e */
Error duplicate_err (Error e)
{
    return mkerr (e.errcode, e.flavour);
}

