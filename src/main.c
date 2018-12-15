/*
 * A simple S-Expression interpreter.
 *
 * Uses a REPL (Read, Eval, Print Loop), which reads user input,
 * evaluates this input, then prints the result of the eval and
 * loops back to Read.
 *
 */

#include "string.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"
#include "print.h"
#include "builtins.h"
#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>



char const *const PROMPT = "> ";

String getinput (char const *prompt);
void init_readline(void);
char **lisp_command_complete (char const *text, int start, int end);
char *lisp_command_generator (char const *text, int state);

Var *get_default_env(void);



/* an s-expression interpreter */
int main (int argc, char *argv[]) {

    GC_init();

    init_readline();
    using_history();

    /* we use a custom printf format (%v) for LISP Vars,
     * so we need to register it with printf */
    init_print();


    /* the default_env contains the primitive functions,
     * such as `+', `define', `if', etc. */
    Var *default_env = get_default_env();


    /* Loop: read input from the user, evaluate it,
     * and print the result */
    String input = getinput (PROMPT);


    /* getinput returns NULL for when we reach EOF */
    while (input.chars != NULL) {

        /* convert user's input to a list of tokens */
        Token *tokens = lex (input.chars);
        debug ("done tokenizing\n");

        if (tokens != NULL)
        {
            //for (size_t i = 0; tokens[i].type != TOK_END; ++i)
            //{
            //    printf ("got '");
            //    print_token (tokens[i]);
            //    printf ("'\n");
            //}

            /* Eval: evaluate the tokens */
            Var *parsed = parse (tokens);
            debug ("parsed '%v'", parsed);

            while (parsed != NULL)
            {
                Var *result = eval (car (parsed), default_env);

                /* Print: print the resulting eval list */
                if (printf (": %v\n", result) == -1)
                {   error ("printf error");
                }

                parsed = cdr (parsed);
            }
        }

        /* Read: read the user input */
        input = getinput (PROMPT);
    }

    return EXIT_SUCCESS;
}



/* getinput: read user input and return it as a string */
String getinput (char const *prompt)
{
    String s = mkstring (readline (prompt));

    if (s.chars != NULL)
    {
        /* check if entered string is already in history,
         * so we don't get duplicated entries*/
        int old_pos = where_history();
        int test;
        /* string is already in history, so we remove it,
         * then add it to the front of the list*/
        if ((test = history_search (s.chars, 0)) != -1)
        {
            free_history_entry (remove_history (test));
            add_history (s.chars);
        }
        /* string is not in history, so add it */
        else
        {   add_history (s.chars);
        }
        history_set_pos (old_pos);
    }
    return s;
}


/* init_readline: initialize GNU Readline */
void init_readline(void)
{   rl_readline_name = "LISP";

    rl_attempted_completion_function = lisp_command_complete;
}

/* lisp_command_complete: GNU Readline completion function */
char **lisp_command_complete (char const *text, int start, int end)
{   rl_attempted_completion_over = 1;
    return rl_completion_matches (text, lisp_command_generator);
}

/* lisp_command_generator: GNU Readline command generator */
char *lisp_command_generator (char const *text, int state)
{
    static int len;
    static size_t i;

    if (!state)
    {   i = 0;
        len = strlen (text);
    }

    /* TODO: search through environments for completions */
    return NULL;
}



struct bi
{
    char *id;
    _Function fn;
};

#define BUILTIN(alias, fnname)  \
    { .id=alias,\
      .fn={ .type   =FN_BUILTIN,\
            .builtin={ .fn  =_builtin_ ## fnname,\
                       .name=alias\
    }}}

static const struct bi builtins[] =
 {
    BUILTIN("+", add),
    BUILTIN("-", sub),
    BUILTIN("*", mul),
    BUILTIN("/", div),

    BUILTIN("if", if),

    BUILTIN("lambda", lambda),
    BUILTIN("define", define),

    BUILTIN("set!", set),
 };
#undef BUILTIN

/* get_default_env: get the default environment */
Var *get_default_env(void)
{
    Var *env = var_nil();

    for (size_t i = 0; i < sizeof(builtins)/sizeof(*builtins); ++i)
    {
        Var *fn = var_atom (atm_fn (builtins[i].fn));
        Var *fnname = var_atom (atm_id (mkstring (builtins[i].id)));
        Var *fnpair = var_pair (fnname, fn);
        env = var_pair (fnpair, env);
    }

    debug ("default_env ==> %v", env);

    return env;
}

