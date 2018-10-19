/*
 * Tokenize LISP expressions
 *
 */

#include "data.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


String read_until (String in, size_t start, char const *const stop_chars);

char const *const LISP_SEPARATORS = " \t\n)(";



/* tokenize: recursively split a string into tokens.
 *           tokenize will read*/
List tokenize (String in, size_t *chars_read)
{
    List symbols = { 0 };

    size_t i = 0;
    for (; i < in.len; ++i)
    {
        debug ("char %zi: '%s'", i, in.chars+i);
        if (!isspace (in.chars[i]))
        {
            /* `)' means the end of our current expression, time to exit */
            if (i > 0 && in.chars[i] == ')')
            {   debug ("end of expr");
                i++;
                break;
            }
            /* start of a new expression, or an atom */
            else
            {   symbols.data = realloc (symbols.data,
                                        sizeof(*symbols.data)
                                        * (symbols.len+1));

                /* `(' means the start of a new expression */
                if (in.chars[i] == '(')
                {   debug ("begin new expr");
                    /* the new expression begins at i, so we must
                     * create a new string to pass to tokenize */
                    String subexpr = NULL_STRING;
                    subexpr.len    = in.len  - (i+1);
                    subexpr.size   = in.size - (i+1);
                    subexpr.chars  = strndup (in.chars+i+1,
                                             subexpr.len);

                    size_t jump_chars = 0;
                    symbols.data[symbols.len].list =\
                        tokenize (subexpr,
                                  &jump_chars);
                    symbols.data[symbols.len].type = VAR_LIST;

                    free_string (subexpr);

                    i += jump_chars;
                }
                /* other characters mean an atom */
                else
                {
                    String word = read_until (in, i, LISP_SEPARATORS);
                    debug  ("got '%s'", word.chars);
                    if (word.len > 0)
                    {   i += word.len-1;
                        Var *dat  = &symbols.data[symbols.len];
                        dat->type = VAR_SYMBOL;
                        dat->sym  = word;
                    }
                }
                symbols.len++;
            }
        }
    }

    /* return the number of chars read, if requested */
    if (chars_read != NULL)
        *chars_read = i;

    return symbols;
}

/* read_until: read characters from a string until
 *             one of the stop_chars is reached */
String read_until (String in, size_t start, char const *stop_chars)
{
    String word = NULL_STRING;

    size_t end = start;
    for (; end < in.len; ++end)
    {   for (size_t i = 0; i < strlen(stop_chars); ++i)
        {   if (in.chars[end] == stop_chars[i])
            {   goto finished;
            }
        }
    }

finished:
    word.len   = end - start;
    word.size  = word.len + 1;
    if (word.len > 0)
    {   word.chars = strndup (in.chars + start, word.len);
    }
    else
    {   word.chars = NULL;
    }

    return word;
}

