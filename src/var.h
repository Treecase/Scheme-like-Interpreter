/*
 * Variables
 *
 */

#ifndef __VAR_H
#define __VAR_H


/* Var:
 *  A LISP variable
 */
typedef struct Var
{   union
    {   Error      err;
        double     number;
        String     str;
        _Function  fn;
        List       list;
        String     sym;
        Identifier id;
    };
    enum
    {
        VAR_UNDEFINED,
        VAR_ERROR,
        VAR_NUMBER,
        VAR_STRING,
        VAR_FUNCTION,
        VAR_LIST,
        VAR_SYMBOL,
        VAR_IDENTIFIER,
    } type;
} Var;



Var vardup (Var v);
void free_var (Var v);


#endif

