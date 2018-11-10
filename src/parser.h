/*
 * Parser for Lisp
 *
 */

#ifndef __PARSER_H
#define __PARSER_H


#include "lexer.h"
#include "data.h"



Var *parse (Token const *tokens);
void print_value (Var *v);


#endif

