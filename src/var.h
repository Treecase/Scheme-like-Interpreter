/*
 * Variables
 *
 */
/* TODO: switch List to Pair */

#ifndef __VAR_H
#define __VAR_H


#include <stdbool.h>    /* bool */

#define UNDEFINED   (new_var (VAR_UNDEFINED))



/* Var:
 *  A LISP variable
 */
typedef struct Var
{   union
    {   Error      err;
        double     number;
        String     str;
        bool       boolean;
        _Function  fn;
        List       list;
        String     sym;
        Identifier id;
    };
    enum VarType
    {   VAR_UNDEFINED,
        VAR_EMPTY,
        VAR_ERROR,
        VAR_NUMBER,
        VAR_STRING,
        VAR_BOOLEAN,
        VAR_FUNCTION,
        VAR_LIST,
        VAR_SYMBOL,
        VAR_IDENTIFIER,
    } type;
} Var;



Var *new_var (enum VarType t);
Var *duplicate_var (Var *v);


#endif

