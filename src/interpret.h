/*
 * Low-level interpreter stuff
 *
 */

#ifndef _INTERPRET_H
#define _INTERPRET_H


#include "data.h"



void print_data (Data d, char endl);

Data expressionize (char *input, int end);

Data eval (S_Exp expression);

Data call (unsigned long func, unsigned int argc, Data *args);

#endif

