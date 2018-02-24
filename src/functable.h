/*
 * Lookup table for functions and associated data
 *
 */


#ifndef _FUNCTABLE_H
#define _FUNCTABLE_H


#include "data.h"


#define VAR_TABLE_DEFAULT_LEN   50
#define DAT_ERROR               ((Data){ -1, { 0 } })



/* variable lookup table */
struct vartable {
    int  len;
    char **names;
    Data *data;
};

struct vartable  VAR_TABLE;



Data symbol_lookup (char *symbol);

int add_symbol_v (struct vartable  *table, char *symbol, Data value);

void dealloc_vartable  (struct vartable  *table);


#endif

