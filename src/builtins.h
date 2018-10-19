/*
 * LISP Builtin functions
 *
 * ie + - * /, etc.
 *
 */

#ifndef __BUILTINS_H
#define __BUILTINS_H

#include "data.h"


#define _GETBUILTINNAME(NAME)   _builtin_ ## NAME
#define MKBUILTIN(NAME)         Var _GETBUILTINNAME(NAME)\
                                (List argv,\
                                 Environment *env)



MKBUILTIN(add);
MKBUILTIN(sub);
MKBUILTIN(mul);
MKBUILTIN(div);

MKBUILTIN(define);

MKBUILTIN(lambda);


#ifndef _NO_UNDEF_MKDEFINE
#undef MKBUILTIN
#undef _GETBUILTINNAME
#endif
#endif

