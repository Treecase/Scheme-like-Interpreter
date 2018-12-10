/*
 * Lexer for Lisp
 *
 */

#include "data.h"
#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



char const *const special_char = ".()|'";
char const *const special_nonnumeric_char = "()|'";

int is_special (char ch);
int is_special_nonnumeric (char ch);



/* Special Characters are `.', `(', and `)'.
 * Seperators are <Special Characters>, ` ', `\t', and `\n'.
 * All other characters are just atoms.
 */
Token *lex (char const *input)
{
    /* The method for lexing is this:
     *  1. iterate throught the input
     *  2. if a Special Character is found, push it onto the output.
     *  3. when a non-Special, non-Whitespace character is encountered,
     *     continue reading until the next Special or Whitespace
     *     character. push this string onto the output.
     *  4. GOTO 1.
     */

    size_t output_len = 0;
    Token *output = calloc ((output_len+1), sizeof(*output));

    for (size_t i = 0; input[i] != '\0'; ++i)
    {
        char ch = input[i];
        if (is_special (ch))
        {
            output = realloc (output, (output_len+1)*sizeof(*output));
            output[output_len] = (Token){ 0 };

            switch (ch)
            {
            case '(':
                output[output_len].type = TOK_LPAREN;
                break;
            case ')':
                output[output_len].type = TOK_RPAREN;
                break;
            case '.':
                output[output_len].type = TOK_DOT;
                break;

            case '|':
                output[output_len].type = TOK_VBAR;
                break;

            case '\'':
                output[output_len].type = TOK_QUOTE;
                break;

            default:
                error ("`%c' is not a special character\n", ch);
                break;
            }

            output_len++;
        }
        else if (!isspace (ch))
        {
            output = realloc (output, (output_len+1)*sizeof(*output));
            output[output_len] = (Token){ 0 };

            size_t begin = i;
            while (input[i] != '\0' && !isspace (input[i])\
                   && !is_special_nonnumeric (input[i]))
            {   i++;
            }
            output[output_len].value = strndup (input+begin, i-begin);
            output[output_len].type  = TOK_TOKEN;
            output_len++;
            i--;
        }
    }

    /* delimit the output with NULL */
    output = realloc (output, (output_len+1)*sizeof(*output));
    output[output_len].type = TOK_END;
    return output;
}


/* is_special: T if ch is a Special Character, else F */
int is_special (char ch)
{
    for (size_t i = 0; special_char[i] != '\0'; ++i)
    {   if (ch == special_char[i])
        {   return 1;
        }
    }
    return 0;
}

/* is_special_nonnumeric: T if ch is a Special Character which
 *  is not used in Number literals, else F */
int is_special_nonnumeric (char ch)
{
    for (size_t i = 0; special_char[i] != '\0'; ++i)
    {   if (ch == special_nonnumeric_char[i])
        {   return 1;
        }
    }
    return 0;
}


/* print_token: */
void print_token (Token t)
{
    switch (t.type)
    {
    case TOK_TOKEN:
        printf ("%s", t.value);
        break;
    case TOK_LPAREN:
        printf ("LPAREN");
        break;
    case TOK_RPAREN:
        printf ("RPAREN");
        break;
    case TOK_DOT:
        printf ("DOT");
        break;
    case TOK_VBAR:
        printf ("VBAR");
        break;
    case TOK_QUOTE:
        printf ("QUOTE");
        break;
    case TOK_END:
        printf ("<END>(hey, what's this doing here?)");
    }
}

