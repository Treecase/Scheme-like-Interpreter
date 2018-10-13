/*
 * Data defs
 *
 */

#ifndef __DATA_H
#define __DATA_H

#include "string.h"
#include "error.h"
#include "token.h"
#include "environment.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>


/* TODO: make this configurable */
#define DEBUG_FILE  stdout

#define fatal(format, ...)  ({  fprintf (stderr, format "\n", ##__VA_ARGS__); exit (EXIT_FAILURE);  })
#define error(format, ...)  ({  fprintf (stderr, format "\n", ##__VA_ARGS__);   })

#ifdef _DEBUG_BUILD
#define debug(format, ...)  ({  fprintf (DEBUG_FILE, format "\n", ##__VA_ARGS__);   })
#else
#define debug(format, ...)
#endif


/* List:
 *  List of Vars
 */
typedef struct List
{   size_t      len;
    struct Var *data;
} List;


/* LISPFunction:
 *  A non-builtin function -- essentially
 *  just a saved token list
 */
typedef struct LISPFunction
{   Token       *body;
    Environment *env;
} LISPFunction;


/* BuiltIn:
 *  Builtins are is basically just
 *  C function pointers
 */
typedef struct BuiltIn
{   /* pointer to a function taking an int
     * and a const pointer to a const Var */
    struct Var (*fn)(size_t, struct Var const *const, Environment *);
} BuiltIn;


/* Function:
 *  A Function can be either a
 *  LISPFunction or a BuiltIn
 *
 *  NOTE: named _Function because
 *  GNU Readline has a _PUBLIC_
 *  typedef for Function already
 *  (... v_v)
 */
typedef struct _Function
{   union
    {   LISPFunction fn;
        BuiltIn      builtin;
    };
    enum
    {   FN_LISPFN,
        FN_BUILTIN,
    } type;
} _Function;


#include "var.h"


#endif

