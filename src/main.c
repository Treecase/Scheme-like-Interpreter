/*
 * A simple S-Expression interpreter.
 *
 * Uses a REPL (Read, Eval, Print Loop), which reads user input,
 * evaluates this input, then prints the result of the eval and
 * loops back to Read.
 *
 */
/* TODO: add a garbage collector instead of allocing everything and just freeing at the end */

#include "string.h"
#include "token.h"
#include "eval.h"
#include "print.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>



char const *const PROMPT = "> ";
Environment *global_env = { 0 };
Environment *local_env = NULL;


String getinput (char const *prompt);
void init_readline(void);
char **lisp_command_complete (char const *text, int start, int end);
char *lisp_command_generator (char const *text, int state);



/* an s-expression interpreter */
int main (int argc, char *argv[]) {

    init_readline();

    /* we use a custom printf format (%v) for LISP Vars,
     * so we need to register it with printf */
    init_print();


    /* the global_env contains program-scope variable names,
     * (eg +, -, define, etc.) */
    global_env = get_default_environment();
    local_env = global_env;


    /* Loop: read input from the user, evaluate it,
     * and print the result */
    String input = getinput (PROMPT);

    while (input.chars != NULL) {

        /* convert user's input to a list of tokens */
        List tokens = tokenize (input, NULL);
        debug ("done tokenizing\n");

        if (tokens.len != 0)
        {
            /* Eval: evaluate the tokens, stuffing
             *       the results into a list */
            Var v = (Var){ .type=VAR_LIST,
                           .list=(List){ .len=0,
                                         .data=NULL }
                            };
            v.list.len = tokens.len;
            v.list.data = calloc (v.list.len, sizeof(*v.list.data));

            for (size_t i = 0; i < tokens.len; ++i)
            {
                v.list.data[i] = eval (tokens.data[i], global_env);
            }

            /* Print: print the resulting eval list */
            if (printf (": %v\n", &v) == -1)
            {   error ("printf error");
            }
            free_var (v);
        }

        /* clean up the memory allocated for this loop */
        for (size_t i = 0; i < tokens.len; ++i)
        {   free_var (tokens.data[i]);
        }
        free (tokens.data);
        tokens.data = NULL;
        tokens.len  = 0;

        /* Read: read the user input */
        free_string (input);
        input = getinput (PROMPT);
    }

    free_string (input);
    free_env (global_env);

    return EXIT_SUCCESS;
}

/* getinput: read user input and return it as a string */
String getinput (char const *prompt)
{
    String s;
    s.chars = NULL;
    s.len   = 0;
    s.size  = 0;

    s.chars = readline (prompt);

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


        s.len  = strlen (s.chars);
        s.size = s.len + 1;
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

    while (i < local_env->len)
    {   /* TODO: search through all Environments for completions */
        i += 1;
        if (strncasecmp (local_env->names[i-1].chars, text, len) == 0)
        {   return strdup (local_env->names[i-1].chars);
        }
    }
    return NULL;
}

