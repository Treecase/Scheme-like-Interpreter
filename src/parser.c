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
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>


#ifdef _DEBUG_BUILD
#define rprintf(format, ...)   printf ("%*s" format, depth, "", ##__VA_ARGS__);
#else
#define rprintf(format, ...)
#endif
#define DEPTH_INC   4


static jmp_buf err_context;
static bool error_occurred = false;
static Var *errmsg = NULL;

static bool parser_at_EOF = false;

static int depth = 0;

Token const *tokens = NULL;


void append (Var *list, Var *end);



/* token_type: returns a string containing the name of t's type */
static char *token_types (enum TokenType t)
{
    switch (t)
    {
    case TOK_TOKEN:
        return "TOKEN";
    case TOK_LPAREN:
        return "LPAREN";
    case TOK_RPAREN:
        return "RPAREN";
    case TOK_DOT:
        return "DOT";
    case TOK_VBAR:
        return "VBAR";
    case TOK_QUOTE:
        return "QUOTE";
        break;
    case TOK_END:
        return "END";
    }
    return "???";
}



Token next(void)
{   tokens++;
    if (tokens->type != TOK_END)
    {
        rprintf ("next -- '%s'\n", token_types (tokens->type));
    }
    else
    {   debug ("Parser: End of Input\n");
        parser_at_EOF = true;
    }
    return *tokens;
}


Var *parse_atom(void);
Var *parse_expr(void);




/* parse: initiate the parsing process */
Var *parse (Token const *input)
{
    tokens = input;
    depth = -DEPTH_INC;

    Var *result = var_pair (NULL, NULL);

    bool false_alarm = false;

    error_occurred = false;
    errmsg = NULL;


    /* this loops, in order to parse multi-expression programs*/
    while (parser_at_EOF == false)
    {
        /* setup a jump point for if parse_expr fails */
        if (setjmp (err_context) == 0)
        {   append (result, parse_expr());
        }
        /* if an error occurred, longjmp will jump here */
        else
        {   if (!false_alarm)
            {
                error ("an error occurred: %v!", errmsg);
                error_occurred = true;
                false_alarm = false;
                return var_pair (errmsg, NULL);
            }
            else
            {   debug ("false alarm.");
                error_occurred = false;
                false_alarm = false;
                continue;
            }
        }
        if (!error_occurred)
        {   /* IMPORTANT: trigger the setjmp in order to avoid
             * ``some kind of subtle or unsubtle chaos''(setjmp(3):63)
             * if no error actually occurred */
            false_alarm = true;
            longjmp (err_context, 1);
        }
    }
    parser_at_EOF = false;
    return cdr (result);
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

        /* Number */
        if (*end == '\0')
        {   v = var_atom (atm_num (n));
        }
        /* Boolean */
        else if (t.value[0] == '#')
        {   v = var_atom (atm_bool (t.value[1] == 't'));
        }
        /* String */
        else if (t.value[0] == '"' && strlen (t.value) > 1 && t.value[strlen (t.value)-1] == '"')
        {   v = var_atom (atm_str (mknstring (t.value+1, strlen (t.value)-2)));
        }
        /* Identifier */
        else
        {   v = var_atom (atm_id (mkstring (t.value)));
        }

        rprintf ("%s: got '%v'\n", __func__, v);

        next();
    }
    else
    {   error ("got bad token type '%s'", token_types (t.type));
        v = var_nil();
        exit (1);
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
                value = var_nil();
            }
            /* `(<expr>' */
            else
            {
                rprintf ("%s: `(<expr>'\n", __func__);

                Var *expr1 = parse_expr();

                rprintf ("%s: `(<expr> ...'\n", __func__);

                value = var_pair (expr1, NULL);
                Var *expr_n = value;

                while ((tokens->type & (TOK_RPAREN | TOK_DOT | TOK_END)) == 0)
                {
                    expr_n->p.cdr = var_pair (NULL, NULL);

                    Var *tmp = expr_n->p.cdr;

                    expr_n = tmp;
                    expr_n->p.car = parse_expr();
                }
                if (tokens->type == TOK_END)
                {   errmsg = mkerr_var (EC_GENERAL,
                                        "parse error -- hit end of "
                                        "input before pattern was "
                                        "matched");
                    longjmp (err_context, 1);
                }

                /* `(<expr1>... <expr_n>)' */
                if (tokens->type == TOK_RPAREN)
                {   expr_n->p.cdr = var_nil();
                }
                /* `(<expr1>... . <expr_n>' */
                else if (tokens->type == TOK_DOT)
                {   next();
                    expr_n->p.cdr = parse_expr();
                    if (tokens->type != TOK_RPAREN)
                    {   errmsg = mkerr_var (EC_GENERAL,
                                            "parse error -- hit end "
                                            "of input before pattern"
                                            " was matched");
                        longjmp (err_context, 1);
                    }
                }
                /* ??? */
                else
                {   rprintf ("?? -- `(<expr1>... ?'\n");
                }
            }
            next();
        }
        /* `|' */
        else if (tokens->type == TOK_VBAR)
        {
            rprintf ("%s: `|'\n", __func__);
            next();

            Token const *begin = tokens;

            /* | <atom> | */
            while ((tokens->type & (TOK_VBAR | TOK_END)) == 0)
            {   rprintf ("<atom> ==> %s\n", tokens->value);
                next();
            }
            if (tokens->type == TOK_END)
            {   errmsg = mkerr_var (EC_GENERAL, "parser error -- "
                                    "hit end of input before "
                                    "pattern was matched");
                longjmp (err_context, 1);
            }

            rprintf ("%s: got `| <atom> |'\n", __func__);
            String id = NULL_STRING;
            for (int i = 0; begin+i < tokens; ++i)
            {
                if (id.len > 0)
                {   id = stringapp (id, mkstring (" "));
                }
                String tmp = mkstring ((begin+i)->value);
                rprintf ("%i => '%s'\n", i, tmp.chars);
                id = stringapp (id, tmp);
            }
            value = var_atom (atm_id (id));
            next();
        }
        /* '<symbol> */
        else if (tokens->type == TOK_QUOTE)
        {
            rprintf ("%s: <symbol>\n", __func__);
            next();
            value = var_atom (atm_sym (parse_expr()));
        }
        /* <atom> */
        else if (tokens->type == TOK_TOKEN)
        {
            rprintf ("%s: <atom>\n", __func__);
            value = parse_atom();
        }
        /* invalid input */
        else
        {   errmsg = mkerr_var (EC_GENERAL, "parser error --"
                                " input matches no patterns");
            longjmp (err_context, 1);
        }
    }
    /* EOI */
    else
    {   rprintf ("END OF INPUT\n");
        parser_at_EOF = true;
        value = var_nil();
    }

    rprintf ("%s: got '%v'\n", __func__, value);

    depth -= DEPTH_INC;
    return value;
}





/* append: add `end' to the end of `list' */
void append (Var *list, Var *end)
{
    if (cdr (list) == NULL)
    {
        list->p.cdr = var_pair (end, NULL);
    }
    else
    {   append (cdr (list), end);
    }
}

