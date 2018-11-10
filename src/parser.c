/*
 * Parser for Lisp
 *
 *
 *
 * Parse rules:
 * datum:
 *  | ()                          ==> nil
 *  | <atom>                      ==> <atom>
 *  | <expr>                      ==> <value of expr>
 *
 * expr:
 *    (<expr>)                    ==> (<expr>.NIL)
 *  | (<expr> . <expr>)           ==> (<expr>.<expr> )
 *  | (<expr> <expr>...)          ==> (<expr>.(...(<expr>.NIL)...))
 *  | (<expr> <expr>... . <expr>) ==> (<expr>.(...(<expr.<expr>)...))
 *
 */

#include "parser.h"
#include "lexer.h"
#include "data.h"

#include <stdio.h>
#include <stdlib.h>


#define rprintf(format, ...)   printf ("%*s" format, depth, "", ##__VA_ARGS__);
#define DEPTH_INC   4


static int depth = 0;

Token const *tokens = NULL;

Token next(void)
{   if (tokens->type != TOK_END)
    {   tokens++;
        rprintf ("next -- '%s'\n", TOKEN_TYPES[tokens->type]);
    }
    else
    {   fprintf (stderr, "Parser: End of Input\n");
        exit (1);
    }
    return *tokens;
}

#define peek()  (*tokens)


Var *parse_atom(void);
Var *parse_expr(void);



/* parse: initiate the parsing process
 * TODO: copy input to `tokens' instead of assigning it
 */
Var *parse (Token const *input)
{
    tokens = input;
    depth = -DEPTH_INC;
    return parse_expr();
}


/* parse_atom: parse <atom>s */
Var *parse_atom(void)
{
    Var *v = NULL;
    Token t = *tokens;

    if (t.type == TOK_TOKEN)
    {
        char *end = NULL;
        double n = strtod (t.value, &end);
        if (*end == '\0')
        {   v = new_atom_num (n);
        }
        else
        {
            v = new_atom_iden (t.value);
        }

        rprintf ("%s: got '", __func__);
        print_value (v);
        printf ("'\n");

        next();
    }
    else
    {   fprintf (stderr, "Error: %s -- got bad token type '%s'\n",
                 __func__, TOKEN_TYPES[t.type]);
        v = new_nil();
        exit(1);
    }

    return v;
}

/* parse_expr: parse <expr>s */
Var *parse_expr(void)
{
    depth += DEPTH_INC;

    Var *value = NULL;


    /* non-empty */
    if (tokens->type != TOK_END)
    {
        /* `(' */
        if (tokens->type == TOK_LPAREN)
        {
            rprintf ("%s: `('\n", __func__);

            next();

            /* `()' */
            if (tokens->type == TOK_RPAREN)
            {
                rprintf ("%s: NIL\n", __func__);

                value = new_nil();
            }
            /* `(<expr>' */
            else
            {
                rprintf ("%s: `(<expr>'\n", __func__);

                Var *expr1 = parse_expr();

                rprintf ("%s: `(<expr> ...'\n", __func__);

                value = new_pair (expr1, NULL);
                Var *expr_n = value;

                while (tokens->type != TOK_RPAREN && tokens->type != TOK_DOT)
                {
                    expr_n->p.cdr = new_pair (NULL, NULL);

                    Var *tmp = expr_n->p.cdr;

                    expr_n = tmp;
                    expr_n->p.car = parse_expr();
                }

                /* `(<expr1>... <expr_n>)' */
                if (tokens->type == TOK_RPAREN)
                {   expr_n->p.cdr = new_nil();
                }
                /* `(<expr1>... . <expr_n>)' */
                else if (tokens->type == TOK_DOT)
                {   next();
                    expr_n->p.cdr = parse_expr();
                }
                /* ??? */
                else
                {   rprintf ("?? -- `(<expr1>... ?'\n");
                }
            }
            next();
        }
        /* <atom> */
        else if (tokens->type == TOK_TOKEN)
        {
            rprintf ("%s: <atom>\n",__func__);
            value = parse_atom();
        }
    }
    /* EOI */
    else
    {
        rprintf ("END OF INPUT\n");
    }

    rprintf ("%s: got '", __func__);
    print_value (value);
    printf ("'\n");

    depth -= DEPTH_INC;
    return value;
}

