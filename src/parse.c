/*
 * Convert input to Number, String, etc.
 *
 */
/* TODO: mark tokens with `possible fail' flags, so we can output helpful
 *       error messages if you accidentally type something like `(+10 10)'
 *       or `(display "Hello World)' */

#include "parse.h"
#include "data.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



/* parse_lexeme: converts from a string to a LISP value.
 *               for example, given `12.3', returns the Number `12.3`;
 *               given `"Hello World"', returns the String "Hello World". */
Token parse_lexeme (String in)
{
    Token out = { .type=TOK_NUMBER, .number=-1 };

    bool failed = false;

    /* Strings start with ", so if the word begins
     * with ", we assume it's a String. */
    if (in.chars[0] == '"')
    {
        size_t i;
        bool found_endquote = false;
        for (i = 1; i < in.len; ++i)
        {   if (in.chars[i] == '"')
            {   found_endquote = true;
                i -= 1;
                break;
            }
        }

        if (found_endquote)
        {   out.type      = TOK_STRING;
            out.str.chars = strndup (in.chars+1, i);
            out.str.len   = i;
            out.str.size  = i+1;
        }
        else
        {   failed = true;
            /* TODO: error - unbalanced quotes */
        }

    }
    /* Numbers start with a digit, a sign, or a decimal, so if the word
     * starts with [1-9], +, -, or ., we assume it's a Number */
    else if (isdigit (in.chars[0]) || in.chars[0] == '+' || in.chars[0] == '-' || in.chars[0] == '.')
    {
        char *end;
        out.type   = TOK_NUMBER;
        out.number = strtod (in.chars, &end);

        /* strtod stores a pointer to the last character used in conversion in `end'.
         * if the word is a valid Number, the entire thing should be used by strtod.
         * if strtod did not use all the characters in the word, then the word must
         * be either an Identifier or an invalid Number literal */
        if (end != in.chars + in.len)
        {   failed = true;
            /* TODO: error - invalid Number literal */
        }
    }
    else
    {   failed = true;
    }

    /* Identifiers can be any sequence of characters, so if the word isn't
     * a Number, and it isn't a String, then it must be an Identifier!
     * (or a malformed Number/String, but we can't really detect that here) */
    if (failed)
    {   out.type     = TOK_IDENTIFIER;
        out.id.chars = strndup (in.chars, in.len);
        out.id.len   = in.len;
        out.id.size  = in.size;
    }
    return out;
}

