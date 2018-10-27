/*
 * Tokenize LISP expressions
 *
 */
/* TODO: comments, '`', ',' and ',@', '\' (pg 9) */

#include "data.h"
#include "global-state.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


String read_until (String in, size_t start, char const *stop_chars);
Var *parse (String in, size_t *num_read);

char const *const DELIMITER = " \t\n)(";



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
                    String the_rest = mkstring (in.chars+i);
                    if (the_rest.len > 0)
                    {   size_t chars_read;

                        symbols->list.data[symbols->list.len] =\
                            parse (the_rest, &chars_read);

                        debug ("got parse '%.*s'",
                               chars_read,
                               the_rest.chars);
                        i += chars_read - 1;
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
Var *parse (String in, size_t *chars_read)
{
    Var *out = UNDEFINED;

    String _word = read_until (in, 0, DELIMITER);

    char first_char = in.chars[0];

    /* Quoted values begin with ' */
    if (first_char == '\'')
    {
        String word = read_until (in, 0, DELIMITER);

        debug ("quoted value '%s'", word.chars);
        out->type = VAR_SYMBOL;
        out->sym  = mkstring (word.chars+1);

        *chars_read = word.len;
    }
    /* Strings start with ", so if the word begins
     * with ", we assume it's a String. */
    else if (first_char == '"')
    {
        String word = read_until (in, 1, "\"");

        *chars_read = 2 + word.len;

        debug ("string '%s'", word.chars);

        if (word.chars[word.len] != '"')
        {   out->type = VAR_STRING;
            out->str  = word;
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
    else if (IS_NUMBER(_word))
#undef IS_NUMBER
    {   debug ("number '%s'", _word.chars);
        char *end;
        out->type   = VAR_NUMBER;
        out->number = strtod (_word.chars, &end);

        *chars_read = _word.len;

        /* strtod stores a pointer to the last character used
         * in conversion in `end'. If the word is a valid Number,
         * the entire thing should be used by strtod. If strtod
         * did not use all the characters in the word, then the
         * word must be either an Identifier or an invalid
         * Number literal */
        if (end != _word.chars + _word.len)
        {   return mkerr_var (EC_BAD_SYNTAX,
                              "invalid Number literal '%s'",
                              _word.chars);
        }
    }
    /* # directives include Booleans, Vectors, Chars, etc. */
    else if (first_char == '#')
    {
        /* Booleans are `#t', `#true', `#f', and `#false' */
        if (stringcmp (_word, mkstring ("#t")) == 0\
         || stringcmp (_word, mkstring ("#true")) == 0)
        {   out->boolean = true;
            out->type = VAR_BOOLEAN;
        }
        else if (stringcmp (_word, mkstring ("#f")) == 0\
              || stringcmp (_word, mkstring ("#false")) == 0)
        {   out->boolean = false;
            out->type = VAR_BOOLEAN;
        }
        /* TODO: Character, Vector, ByteVector, Numbers, Labels */
        else
        {   return mkerr_var (EC_BAD_SYNTAX,
                              "invalid # directive '%s'",
                              _word.chars);
        }
        *chars_read = _word.len;
    }
    /* Identifiers can be any sequence of characters, so
     * if all else fails, it must be an identifier */
    else
    {
        out->type = VAR_IDENTIFIER;

        /* an identifier can be enclosed in vertical bars,
         * this allows for whitespace in Identifiers */
        if (first_char == '|')
        {
            String id   = read_until (in, 1, "|");
            out->id     = id;
            *chars_read = 2 + id.len;
        }
        /* no vbar, regular Identifier */
        else
        {   out->id     = _word;
            *chars_read = _word.len;
        }
        debug ("identifier '%s'", out->id.chars);
    }
    debug ("got %zi chars", *chars_read);

    return out;
}

