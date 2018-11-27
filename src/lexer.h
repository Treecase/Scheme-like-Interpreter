/*
 * Lexer for Lisp
 *
 */

#ifndef __LEXER_H
#define __LEXER_H



/* Token:
 *  token
 */
typedef struct Token
{   char *value;
    enum
    {   TOK_TOKEN  = 0x01,
        TOK_LPAREN = 0x02,
        TOK_RPAREN = 0x04,
        TOK_DOT    = 0x08,
        TOK_VBAR   = 0x10,
        TOK_END    = 0x20
    } type;
} Token;

static char const *const TOKEN_TYPES[] =
  { "TOKEN",
    "LPAREN",
    "RPAREN",
    "DOT",
    "END"
  };


Token *lex (char const *input);
void print_token (Token t);


#endif

