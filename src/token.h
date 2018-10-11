/*
 * Tokenize LISP expressions
 *
 */

#ifndef __TOKEN_H
#define __TOKEN_H

#include "string.h"


/* Token:
 *  A token can be either an atom (such as a string or number)
 *  or a sub-expression (as in `(* (+ 3 2) 5)')
 */
typedef struct Token
{   union
    {
        double        number;
        String        str;
        Identifier    id;
        struct Token *subexpr;
    };
    enum
    {   TOK_NUMBER,
        TOK_STRING,
        TOK_IDENTIFIER,
        TOK_EXPR,
        TOK_ENDEXPR,
    } type;
} Token;

Token *tokenize (String in, size_t *chars_read);
void free_tokens (Token *tokens);


#endif
