/*
 * Tokenize LISP expressions
 *
 */

#include "data.h"
#include "parse.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include <stdio.h>
#include <assert.h>


String read_until (String in, size_t *start, char const *const stop_chars);

char const *const LISP_SEPARATORS = " \t\n)(";



/* tokenize: recursively split a string into tokens.
 *           tokenize will read*/
Token *tokenize (String in, size_t *chars_read)
{
    Token *tokens = NULL;
    size_t num_tokens = 0;

    size_t i;
    for (i = 0; i < in.len; ++i)
    {
        if (!isspace (in.chars[i]))
        {
            /* `)' means the end of our current expression, time to exit */
            if (i > 0 && in.chars[i-1] == ')')
            {   break;
            }
            /* start of a new expression, or an atom */
            else
            {
                tokens = realloc (tokens, sizeof(*tokens) * (num_tokens+1));

                /* `(' means the start of a new expression */
                if (in.chars[i] == '(')
                {
                    /* the new expression begins at i, so we must
                     * create a new string to pass to tokenize */
                    String subexpr;
                    subexpr.len   = in.len  - (i+1);
                    subexpr.size  = in.size - (i+1);
                    subexpr.chars = strndup (in.chars+i+1, subexpr.len);

                    size_t subexpr_length;
                    tokens[num_tokens].subexpr = tokenize (subexpr,
                                                           &subexpr_length);
                    tokens[num_tokens].type    = TOK_EXPR;

                    free_string (subexpr);

                    i += subexpr_length;
                }
                /* other characters mean an atom */
                else
                {   /* an atom ends once we hit whitespace or a close bracket */
                    String lexeme = read_until (in, &i, LISP_SEPARATORS);
                    if (lexeme.len > 0)
                    {   debug ("got '%s'", lexeme.chars);
                        tokens[num_tokens] = parse_lexeme (lexeme);
                    }
                    free_string (lexeme);
                }
                num_tokens++;
            }
        }
    }

    if (tokens != NULL)
    {   /* we have to mark the end of the expression */
        tokens = realloc (tokens, sizeof(*tokens) * (num_tokens + 1));
        tokens[num_tokens] = (Token){ .type=TOK_ENDEXPR };
    }
    /* return the number of chars read, if requested */
    if (chars_read != NULL)
        *chars_read = i+1;

    return tokens;
}

/* read_until: read characters from a string until
 *             one of the stop_chars is reached */
String read_until (String in, size_t *start, char const *const stop_chars)
{
    String word = NULL_STRING;

    size_t end;
    for (end = *start; end < in.len; ++end)
    {   for (size_t i = 0; i < strlen(stop_chars); ++i)
        {   if (in.chars[end] == stop_chars[i])
            {   goto finished;
            }
        }
    }

finished:
    word.len   = end - (*start);
    word.size  = word.len + 1;
    if (word.len > 0)
    {   word.chars = strndup (in.chars + *start, word.len);
    }
    else
    {   word.chars = NULL;
    }

    *start = end;

    return word;
}

/* free_tokens: recursively free a token list */
void free_tokens (Token *tokens)
{
    if (tokens != NULL)
    {   for (size_t i = 0; tokens[i].type != TOK_ENDEXPR; ++i)
        {
            switch (tokens[i].type)
            {
            case TOK_STRING:
                free_string (tokens[i].str);
                break;
            case TOK_IDENTIFIER:
                free_string (tokens[i].id);
                break;

            case TOK_EXPR:
                free_tokens (tokens[i].subexpr);
                break;

            /* these don't alloc memory */
            case TOK_NUMBER:
            case TOK_ENDEXPR:
                break;
            }
        }
        free (tokens);
    }
}

