/*
 * Low-level interpreter stuff
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "interpret.h"
#include "data.h"
#include "functable.h"
#include "primitive.h"



/* print_data: print value of a data */
void print_data (Data d, char endl) {

    switch (d.type) {
    case T_INTEGER:
        printf ("%i%c", d.value.ival, endl);
        break;
    case T_FLOAT:
        printf ("%f%c", d.value.fval, endl);
        break;
    case T_STRING:
        printf ("%s%c", d.value.str, endl);
        break;
    case T_FUNC:
        printf ("%s%c", FUNC_TABLE.func[d.value.func].name, endl);
        break;
    case T_VARIABLE:
        print_data (VAR_TABLE.data[d.value.vari], endl);
        break;
    case T_ERROR:
        printf ("ERROR%c", endl);
        break;
    default:
        printf ("Unknown datatype (%i)%c", d.type, endl);
        break;
    }
}

/* expressionize: convert a string into an s-expression */
Data expressionize (char *string, int end) {

    S_Exp exp;
    int   data_i      = 0;
    exp.len           = 3;
    exp.data          = calloc (1, sizeof(Data) * exp.len);

    int   sstr_i      = 0;
    int   sstr_len    = 10;
    char  *substring  = calloc (sstr_len, 1);

    static int offset = 0;

    char c;
    int i, l;
    i = l = 0;

    if (string[0] == '(')
        i = 1;

    for (; string[i-1] != ')' && i < end; ++i, ++offset) {

        c = string[i];

        // eval subexpression + append
        if (c == '(') {
            exp.data[data_i++] = expressionize (string+i+1, end);
            if (data_i >= exp.len) {
                exp.len *= 2;
                exp.data = realloc (exp.data, sizeof(Data) * exp.len);
            }
            i += offset-1;
            l++;
        }
        /* eval substring function
            + clear substring */
        else if (isspace (c) || c == ')' || c == EOF) {
            //printf ("add %s\n", substring);

            char *f = NULL;
            if (exp.data[0].type == T_FUNC
             && exp.data[0].value.func < FUNC_TABLE.len)
                f = FUNC_TABLE.func[exp.data[0].value.func].base;
            // check for def'ing
            if (data_i != 1 || f == NULL || strcmp (f, "P_DEF"))
                exp.data[data_i++] = symbol_lookup (substring);
            else
                exp.data[data_i++] = (Data){ T_STRING,
                    { .str=strdup (substring) } };

            if (data_i >= exp.len) {
                exp.len *= 2;
                exp.data = realloc (exp.data, sizeof(Data) * exp.len);
            }
            memset (substring, 0, sstr_len);
            sstr_i = 0;
            l++;
        }
        // add to substring
        else {
            substring[sstr_i++] = c;
            if (sstr_i >= sstr_len) {
                sstr_len *= 2;
                substring = realloc (substring, sstr_len);
            }
        }
    }
    if (i == end-1)
        offset = 0;
    exp.len = l;

    //printf ("args: ");
    //for (int n = 0; n < data_i; ++n)
    //    print_data (exp.data[n], ' ');
    putchar ('\n');

    free (substring);

    return eval (exp);
}

/* eval: evaluate an s-expression */
Data eval (S_Exp exp) {

    Data rval = DAT_ERROR;
    int num_args = FUNC_TABLE.func[exp.data[0].value.func].argc;

    if (exp.len == 1)
        rval = exp.data[0];
    else if (exp.len <= 0) {
        printf ("invalid arg count %i\n");
    }
    // error if first value is not a function
    else if (exp.data[0].type != T_FUNC) {
        printf ("Error: object ");
        print_data (exp.data[0], 0);
        puts (" is not a function!");
    }
    // check for proper arg count
    else if (num_args != 0 && exp.len > 0 && exp.len-1 > num_args) {
        printf ("Error: invalid no. of arguments - takes %i, given %i\n",
            num_args, exp.len);
    }
    else {
        //printf ("Calling %s\n",
        //    FUNC_TABLE.func[exp.data[0].value.func].name);
        rval = call (exp.data[0].value.func, exp.len, exp.data+1);
    }

    free (exp.data);

    return rval;

}

/* call: calls a function */
Data call (unsigned long func, unsigned int argc, Data *args) {

    char *base = FUNC_TABLE.func[func].base;

    double (*apply)(double, double) = NULL;

    // primitive functions
         if (!strcmp (base, "P_ADD"))
        apply = p_add;
    else if (!strcmp (base, "P_SUB"))
        apply = p_sub;
    else if (!strcmp (base, "P_MUL"))
        apply = p_mul;
    else if (!strcmp (base, "P_DIV"))
        apply = p_div;
    else if (!strcmp (base, "P_DEF")) {
        if (args[0].type == T_STRING) {
            return (Data){ T_VARIABLE, { .vari=add_symbol_v (&VAR_TABLE,
                args[0].value.str, args[1]) } };
        }
        else {
            printf ("Error: call - arg1 (");
            print_data (args[0], 0);
            puts (") is not a string!");
            return DAT_ERROR;
        }
    }

    // apply primitive function
    if (apply) {
        double rval = args[0].type == T_INTEGER? args[0].value.ival :
                        args[0].type == T_FLOAT? args[0].value.fval :
                        printf ("invalid type: %i\n", args[0].type);
        char is_float = 0;
        for (int i = 1; i < argc-1; ++i)
            switch (args[i].type) {
            case T_FLOAT:
                is_float = 1;
                rval = apply (rval, args[i].value.fval);
                break;
            case T_INTEGER:
                rval = apply (rval, args[i].value.ival);
                break;
            case T_VARIABLE:{
                Data dat = VAR_TABLE.data[args[i].value.vari];
                double val = 0;
                switch (dat.type) {
                case T_INTEGER:
                    val = dat.value.ival;
                    break;
                case T_FLOAT:
                    val = dat.value.fval;
                default:
                    printf ("Error: call - arg#%i invalid operand"
                    " for %s : ", i+1, base+2);
                    print_data (dat, '\n');
                    return DAT_ERROR;
                    break;
                }
                rval = apply (rval, val); }
            default:
                printf ("Error: arg #%i - invalid operand for %s : ",
                    i+1, base+2);
                print_data (args[i], '\n');
                return DAT_ERROR;
                break;
            }
        //printf ("primitive returning %f\n\n", rval);
        if (is_float)
            return (Data){ T_FLOAT,   { .fval=rval } };
        else
            return (Data){ T_INTEGER, { .ival=rval } };
    }

    // create a string
    char *fcall = calloc (1, FUNC_TABLE.func[func].baselen+1);
    memcpy (fcall, FUNC_TABLE.func[func].base,
        FUNC_TABLE.func[func].baselen);
    puts (fcall);

    Data rval = expressionize (fcall, strlen (fcall));

    free (fcall);

    return rval;
}

