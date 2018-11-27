/*
 * LISP Builtin functions
 *
 * ie +, -, *, /, etc.
 *
 */

#ifndef __BUILTINS_H
#define __BUILTINS_H

#include "data.h"


#define _GETBUILTINNAME(NAME)   _builtin_ ## NAME
#define MKBUILTIN(NAME)         Var * _GETBUILTINNAME(NAME)\
                                (Var *argv, Var *env)



MKBUILTIN(add);
MKBUILTIN(sub);
MKBUILTIN(mul);
MKBUILTIN(div);

MKBUILTIN(lambda);
MKBUILTIN(define);

MKBUILTIN(if);

MKBUILTIN(set);

MKBUILTIN(include);
MKBUILTIN(include_ci);


Var *atom (Var *x);
Var *eq (Var *x, Var *y);

Var *car (Var *p);
Var *cdr (Var *p);
Var *cons (Var *a, Var *b);


#ifndef _NO_UNDEF_MKDEFINE
#undef MKBUILTIN
#undef _GETBUILTINNAME
#endif
#endif

