/*
 * Lookup table for functions and associated data
 *
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "functable.h"
#include "data.h"



int s_isnumeric (char *string);

extern struct vartable  VAR_TABLE;
static char vt_init = 0;


/* init_vartable: initialize variable table
    NOTE: called automatically by relevant functions */
void init_vartable() {

    int l = VAR_TABLE_DEFAULT_LEN;

    VAR_TABLE.len   = l;
    VAR_TABLE.names = calloc (l, sizeof(char *));
    VAR_TABLE.data  = calloc (l, sizeof(Data));

    for (int i = 0; i < l; ++i)
        VAR_TABLE.names[i] = NULL;

    // init primitive funcs
    for (int i = 0; i < 5; ++i) {
        VAR_TABLE.data[i].value.func.argc    = 0;   // infinite args
        VAR_TABLE.data[i].value.func.baselen = 5;
    }
    VAR_TABLE.names[0] = strdup ("+");
    VAR_TABLE.data[0].value.func.name = strdup ("+");
    VAR_TABLE.data[0].value.func.base = strdup ("P_ADD");
    VAR_TABLE.names[1]                = strdup ("-");
    VAR_TABLE.data[1].value.func.name = strdup ("-");
    VAR_TABLE.data[1].value.func.base = strdup ("P_SUB");
    VAR_TABLE.names[2]                = strdup ("*");
    VAR_TABLE.data[2].value.func.name = strdup ("*");
    VAR_TABLE.data[2].value.func.base = strdup ("P_MUL");
    VAR_TABLE.names[3]                = strdup ("/");
    VAR_TABLE.data[3].value.func.name = strdup ("/");
    VAR_TABLE.data[3].value.func.base = strdup ("P_DIV");
    VAR_TABLE.names[4]                = strdup ("define");
    VAR_TABLE.data[4].value.func.name = strdup ("define");
    VAR_TABLE.data[4].value.func.base = strdup ("P_DEF");

    vt_init = 1;
}

/* dealloc_vartable: frees vartable stuff */
void dealloc_vartable (struct vartable *table) {
    if (!vt_init)
        return;

    for (int i = 0; i < table->len; ++i) {
        if (table->names[i] != NULL)
            free (table->names[i]);
        if (table->data[i].type == T_STRING
         && table->data[i].value.str != NULL)
            free (table->data[i].value.str);
    }
    free (table->names);
    free (table->data);

    vt_init = 0;
}

/* symbol_lookup: search through the function
    and variable symbol lists */
Data symbol_lookup (char *symbol) {
    if (!vt_init)
        init_vartable();

    if (symbol == NULL) {
        puts ("Error: symbol_lookup - symbol is null!");
        return DAT_ERROR;
    }

    // sym is a number?
    int isnumber = s_isnumeric (symbol);
    if (isnumber == 1)
        return (Data){ T_INTEGER, { .ival=atoi (symbol) } };
    else if (isnumber == 2)
        return (Data){ T_FLOAT,   { .fval=atof (symbol) } };

    // sym is a variable?
    for (int i = 0; i < VAR_TABLE.len; ++i)
        if (VAR_TABLE.names[i]
          && !strcmp (VAR_TABLE.names[i], symbol))
            return VAR_TABLE.data[i];
    // TODO : sym is a string?
    return (Data){ T_STRING, { .str=strdup (symbol) } };

    // not found
    return DAT_ERROR;
}

/* add_symbol_v: add a symbol to the variable table */
int add_symbol_v (struct vartable *table, char *symbol, Data value) {
    if (!vt_init)
        init_vartable();

    int index = -1;

    // sym already in table?
    for (int i = 0; i < table->len && index == -1; ++i)
        if (table->names[i] && !strcmp (symbol, table->names[i]))
            index = i;

    // new sym, search for open slot
    for (int i = 0; index == -1 && i < table->len; ++i)
        if (table->names[i] == NULL)
            index = i;

    // no open slots, resize vartable
    if (index == -1) {
        index = table->len;
        table->len++;
        table->names = realloc (table->names,
            table->len * sizeof(char *));
        table->data  = realloc (table->data,
            table->len * sizeof(Data));
    }
    table->names[index] = strdup (symbol);
    table->data[index]  = value;

    return index;
}

/* s_isnumeric: check whether a string is a number */
int s_isnumeric (char *string) {

    int is_float = 0;
    int i = 0;

    // skip signage
    if (strlen (string) != 1 && (string[0] == '-' || string[0] == '+'))
        i = 1;

    for (; string[i] != 0; ++i) {
        if (string[i] == '.')
            is_float = 1;
        if (!isdigit (string[i]) && string[i] != '.')
            return 0;
    }
    return 1 + is_float;
}

