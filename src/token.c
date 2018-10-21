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
Var parse (String word);

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
                    {
                        i += word.len-1;
                        symbols.data[symbols.len] = parse (word);
                    }
                    free_string (word);
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

/* parse: parse a string into a Var */
Var parse (String word)
{
    Var out = { .type=VAR_UNDEFINED };

    /* Quoted values begin with ' */
    if (word.chars[0] == '\'')
    {   debug ("quoted value '%s'", word.chars);
        out.type = VAR_SYMBOL;
        out.sym  = mkstring (word.chars+1);
    }
    /* Strings start with ", so if the word begins
     * with ", we assume it's a String. */
    else if (word.chars[0] == '"')
    {   debug ("string '%s'", word.chars);
        bool found_endquote = false;

        size_t i = 1;
        for (; i < word.len; ++i)
        {   if (word.chars[i] == '"')
            {   found_endquote = true;
                i -= 1;
                break;
            }
        }

        if (found_endquote)
        {   out.type      = VAR_STRING;
            out.str.chars = strndup (word.chars+1, i);
            out.str.len   = i;
            out.str.size  = i+1;
        }
        else
        {   return mkerr_var (EC_BAD_SYNTAX,
                              "Unbalanced quotes -- '%s'",
                              word.chars);
        }
    }
    /* Numbers start with a digit, a sign, or a decimal, so if
     * the word starts with [1-9], or if it's longer than 1
     * character and begins with +, -, or ., it's a Number. */
#define IS_NUMBER(str)  (isdigit (str.chars[0])\
                        || (str.len > 1\
                         && (str.chars[0] == '+'\
                          || str.chars[0] == '-'\
                          || str.chars[0] == '.')))
    else if (IS_NUMBER(word))
#undef IS_NUMBER
    {   debug ("number '%s'", word.chars);
        char *end;
        out.type   = VAR_NUMBER;
        out.number = strtod (word.chars, &end);

        /* strtod stores a pointer to the last character used
         * in conversion in `end'. If the word is a valid Number,
         * the entire thing should be used by strtod. If strtod
         * did not use all the characters in the word, then the
         * word must be either an Identifier or an invalid
         * Number literal */
        if (end != word.chars + word.len)
        {   return mkerr_var (EC_BAD_SYNTAX,
                                  "invalid Number literal '%s'",
                                  word.chars);
        }
    }
    /* Identifiers can be any sequence of characters, so if all else
     * fails, it must be an identifier */
    else
    {   debug ("identifier '%s'", word.chars);
        out.type = VAR_IDENTIFIER;
        out.sym  = stringdup (word);
    }

    return out;
}

