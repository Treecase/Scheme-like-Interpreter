/*
 * A simple S-Expression interpreter
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "interpret.h"
#include "functable.h"



/* a simple s-expression interpreter */
int main (int argc, char *argv[]) {
    
    // loop
    char quit = 0;
    while (!quit) {

        printf ("> ");

        // read
        char input[1024];// = "(define x 25.0)";
        char c;
        memset (input, 0, 1024);
        for (int i = 0; i < 1024 && (c = getchar()) != EOF; ++i) {
            if (c == '\n')
                break;
            else
                input[i] = c;
        }
        if (c == EOF) {
            puts ("\nExit.");
            break;
        }

        // eval + print
        Data dat = expressionize (input, strlen (input));
        printf ("return ");
        print_data (dat, '\n');
        putchar ('\n');

        if (dat.type == T_STRING && dat.value.str)
            free (dat.value.str);
    }
    dealloc_functable (&FUNC_TABLE);
    dealloc_vartable  (&VAR_TABLE);

    return 0;
}

