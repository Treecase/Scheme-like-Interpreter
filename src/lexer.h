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
    {   TOK_TOKEN,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_DOT,
        TOK_END
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

