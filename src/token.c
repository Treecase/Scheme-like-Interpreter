/*
 * Tokenize LISP expressions
 *
 */

#include "data.h"
#include "global-state.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


String read_until (String in, size_t start, char const *stop_chars);
Var *parse (String word);

char const *const LISP_SEPARATORS = " \t\n)(";



/* tokenize: recursively split a string into tokens.
 *
 * To tokenize, we walk through each character in the string
 * until we hit a key character. For example, a `(' character
 * means the beginning of a new expression. */
Var *tokenize (String in, size_t *chars_read)
{
    Var *symbols       = new_var (VAR_LIST);
    symbols->list.len  = 0;
    symbols->list.data = NULL;


    List *l = &symbols->list;

    size_t i = 0;
    for (; i < in.len; ++i)
    {
        debug ("%zi: '%s'", i, in.chars+i);
        char ch = in.chars[i];
        if (!isspace (ch))
        {
            /* `)' means the end of our current expression,
             * time to exit */
            if (i > 0 && ch == ')')
            {   debug ("end of expr");
                i++;
                break;
            }
            /* start of a new expression, or an atom */
            else
            {
                l->data = GC_realloc (l->data,
                                      (l->len+1)*sizeof(*l->data));

                /* `(' means the start of a new expression */
                if (ch == '(')
                {   debug ("begin new expr");
                    /* the new expression begins at i, so we must
                     * create a new string to pass to tokenize */
                    String subexpr = mknstring (in.chars+i+1,
                                                in.len-(i+1));

                    size_t jump_chars = 0;
                    Var **v = &l->data[l->len];
                    *v = tokenize (subexpr, &jump_chars);

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
                        symbols->list.data[symbols->list.len] = parse (word);
                    }
                }
                symbols->list.len++;
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
    size_t end = start;
    for (; end < in.len; ++end)
    {   for (size_t i = 0; i < strlen(stop_chars); ++i)
        {   if (in.chars[end] == stop_chars[i])
            {   goto finished;
            }
        }
    }

finished:
    return mknstring (in.chars+start, end - start);
}

/* parse: parse a string into a Var */
Var *parse (String word)
{
    Var *out = UNDEFINED;

    char first_char = word.chars[0];

    /* Quoted values begin with ' */
    if (first_char == '\'')
    {   debug ("quoted value '%s'", word.chars);
        out->type = VAR_SYMBOL;
        out->sym  = mkstring (word.chars+1);
    }
    /* Strings start with ", so if the word begins
     * with ", we assume it's a String. */
    else if (first_char == '"')
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
        {   out->type = VAR_STRING;
            out->str  = mknstring (word.chars+1, i-1);
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
        out->type   = VAR_NUMBER;
        out->number = strtod (word.chars, &end);

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
    /* Booleans are represented by `#f' or `#t'
     * for `true' and `false', respectively. */
    else if (word.chars[0] == '#')
    {
        out->type = VAR_BOOLEAN;
        if (word.chars[1] == 'f')
        {   out->boolean = false;
        }
        else if (word.chars[1] == 't')
        {   out->boolean = true;
        }
        else
        {   mkerr_var (EC_BAD_SYNTAX,
                       "Invalid Boolean literal '%s'",
                       word.chars);
        }
    }
    /* Identifiers can be any sequence of characters, so
     * if all else fails, it must be an identifier */
    /* TODO: identifiers in vbars (eg |hello world|) */
    else
    {   debug ("identifier '%s'", word.chars);
        out->type = VAR_IDENTIFIER;
        out->sym  = stringdup (word);
    }

    return out;
}

