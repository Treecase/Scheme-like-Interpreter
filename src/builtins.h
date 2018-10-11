/*
 * LISP Builtin functions
 *
 * ie + - * /, etc.
 *
 */

#ifndef __BUILTINS_H
#define __BUILTINS_H

#include "data.h"


Var _builtin_add (size_t argc, Var const *const argv);
Var _builtin_sub (size_t argc, Var const *const argv);
Var _builtin_mul (size_t argc, Var const *const argv);
Var _builtin_div (size_t argc, Var const *const argv);


#endif

