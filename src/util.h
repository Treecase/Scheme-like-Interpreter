/*
 * Utility functions for LISP
 *
 */

#ifndef __CHEADER_H
#define __CHEADER_H

#include "data.h"



Var *atom (Var *x);
Var *eq (Var *x, Var *y);

Var *car (Var *p);
Var *cdr (Var *p);
Var *cons (Var *a, Var *b);


#endif

