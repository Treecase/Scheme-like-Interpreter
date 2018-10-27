/*
 * A simple S-Expression interpreter.
 *
 * Uses a REPL (Read, Eval, Print Loop), which reads user input,
 * evaluates this input, then prints the result of the eval and
 * loops back to Read.
 *
 */

#include "string.h"
#include "token.h"
#include "eval.h"
#include "print.h"
#include "global-state.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>



/* Globals */
Environment *local_env  = NULL;


char const *const PROMPT = "> ";
Environment *global_env = { 0 };


String getinput (char const *prompt);
void init_readline(void);
char **lisp_command_complete (char const *text, int start, int end);
char *lisp_command_generator (char const *text, int state);



/* an s-expression interpreter */
int main (int argc, char *argv[]) {

    GC_init();

    init_readline();
    using_history();

    /* we use a custom printf format (%v) for LISP Vars,
     * so we need to register it with printf */
    init_print();


    /* the global_env contains program-scope variable names,
     * (eg +, -, define, etc.) */
    global_env = get_default_environment();
    local_env  = global_env;


    /* Loop: read input from the user, evaluate it,
     * and print the result */
    String input = getinput (PROMPT);


    /* getinput returns NULL for when we reach EOF */
    while (input.chars != NULL) {

        /* convert user's input to a list of tokens */
        Var *tokens = tokenize (input, NULL);
        debug ("done tokenizing\n");

        if (tokens != NULL && tokens->list.len != 0)
        {
            debug ("got '%v'", tokens);

            /* Eval: evaluate the tokens, stuffing
             *       the results into a list */
            Var *result = new_var (VAR_LIST);
            result->list.len  = tokens->list.len;
            result->list.data = GC_malloc (result->list.len * sizeof(*result->list.data));

            for (size_t i = 0; i < tokens->list.len; ++i)
            {   result->list.data[i] = eval (tokens->list.data[i], global_env);
            }

            /* Print: print the resulting eval list */
            if (printf (": %v\n", result) == -1)
            {   error ("printf error");
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

    while (i < local_env->len)
    {   /* TODO: search through all Environments for completions */
        i += 1;
        char *str = local_env->names[i-1].chars;
        if (strncasecmp (str, text, len) == 0)
        {   return strdup (str);
        }
    }
    return NULL;
}

