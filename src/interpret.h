/*
 * Low-level interpreter stuff
 *
 */

#ifndef _INTERPRET_H
#define _INTERPRET_H


#include "data.h"



void print_data (Data d, char endl);

Data expressionize (char *input, int end);

Data eval (Data *expression, int len);

Data call (struct func func, unsigned int argc, Data *args);

#endif

