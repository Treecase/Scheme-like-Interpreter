/*
 * Tokenize LISP expressions
 *
 */

#ifndef __TOKEN_H
#define __TOKEN_H

#include "string.h"



struct List tokenize (String in, size_t *chars_read);


#endif
