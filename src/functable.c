/*
 * Lookup table for functions and associated data
 *
 */

#include <stdio.h> // FIXME : temp

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "functable.h"
#include "data.h"



int s_isnumeric (char *string);
void ftable_setup_primitives (struct functable *t);

extern struct functable FUNC_TABLE;
static char ft_init = 0;
extern struct vartable  VAR_TABLE;
static char vt_init = 0;


/* init_functable: initialize function table
    NOTE: called automatically by relevant functions */
void init_functable() {

    int l = FUNC_TABLE_DEFAULT_LEN;
    FUNC_TABLE.len  = l;
    FUNC_TABLE.func = calloc (l, sizeof(struct function));
    for (int i = 0; i < l; ++i) {
        FUNC_TABLE.func[i].name = NULL;
        FUNC_TABLE.func[i].base = NULL;
    }

    ftable_setup_primitives (&FUNC_TABLE);

    ft_init = 1;
}

/* init_vartable: initialize variable table
    NOTE: called automatically by relevant functions */
void init_vartable() {

    int l = VAR_TABLE_DEFAULT_LEN;

    VAR_TABLE.len   = l;
    VAR_TABLE.names = calloc (l, sizeof(char *));
    VAR_TABLE.data  = calloc (l, sizeof(Data));

    for (int i = 0; i < l; ++i)
        VAR_TABLE.names[i] = NULL;

    vt_init = 1;
}

/* ftable_setup_primitives: initialize the functable
    with primitive operators (ie +*-/ etc) */
void ftable_setup_primitives (struct functable *t) {

    // primitives are : +, -, *, /, define

    // infinite args
    for (int i = 0; i < 5; ++i) {
        t->func[i].name   = malloc (2);
        t->func[i].argc    = 0;         // infinite args
        t->func[i].base    = malloc (6);
        t->func[i].baselen = 0;
    }
    strcpy (t->func[0].name, "+");
    strcpy (t->func[0].base, "P_ADD");
    strcpy (t->func[1].name, "-");
    strcpy (t->func[1].base, "P_SUB");
    strcpy (t->func[2].name, "*");
    strcpy (t->func[2].base, "P_MUL");
    strcpy (t->func[3].name, "/");
    strcpy (t->func[3].base, "P_DIV");

    t->func[4].name = realloc (t->func[4].name, 7);
    strcpy (t->func[4].name, "define");
    strcpy (t->func[4].base, "P_DEF");
    t->func[4].argc = 2;  // define takes 2 args

}

/* dealloc_functable: frees functable stuff */
void dealloc_functable (struct functable *table) {
    if (!ft_init)
        return;

    for (int i = 0; i < table->len; ++i) {
        if (table->func[i].name != NULL)
            free (table->func[i].name);
        if (table->func[i].base != NULL)
            free (table->func[i].base);
    }
    free (table->func);

    ft_init = 0;
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
    if (!ft_init)
        init_functable();
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

    // sym is a function?
    for (int i = 0; i < FUNC_TABLE.len; ++i)
        if (FUNC_TABLE.func[i].name
          && !strcmp (FUNC_TABLE.func[i].name, symbol))
            return (Data){ T_FUNC, { .func=i} };
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

/* add_symbol_f: add a symbol to the function table */
int add_symbol_f (struct functable *table, char *symbol) {
    if (!ft_init)
        init_functable();

    int index = -1;

    for (int i = 0; index == -1 && i < table->len; ++i)
        if (table->func[i].name == NULL)
            index = i;

    // resize functable
    if (index == -1) {
        index = table->len;
        table->len++;
        table->func = realloc (table->func,
            table->len * sizeof(struct function));
    }
    int len = strlen (symbol);
    table->func[index].name  = malloc (len+1);
    memcpy (table->func[index].name, symbol, len);
    table->func[index].name[len] = 0;

    return index;
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

