/*
 * Lookup table for functions and associated data
 *
 */


#ifndef _FUNCTABLE_H
#define _FUNCTABLE_H


#include "data.h"


#define FUNC_TABLE_DEFAULT_LEN  50
#define VAR_TABLE_DEFAULT_LEN   50
#define DAT_ERROR               ((Data){ -1, { 0 } })


/* function datatype */
struct function {
    char          *name;
    unsigned long argc;
    char          *base;
    int           baselen;
};

/* function lookup table */
struct functable {
    int             len;
    struct function *func;
};

/* variable lookup table */
struct vartable {
    int  len;
    char **names;
    Data *data;
};

struct functable FUNC_TABLE;
struct vartable  VAR_TABLE;



Data symbol_lookup (char *symbol);

int add_symbol_f (struct functable *table, char *symbol);
int add_symbol_v (struct vartable  *table, char *symbol, Data value);

void dealloc_functable (struct functable *table);
void dealloc_vartable  (struct vartable  *table);


#endif

