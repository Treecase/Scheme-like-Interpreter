/*
 * Environment
 *
 */

#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H

#include "string.h"

#include <stddef.h> /* size_t */


/* Environment:
 *  Associates variable names to values
 */
typedef struct Environment
{   size_t       len;
    Identifier  *names;
    struct Var **values;

    struct Environment *parent;
} Environment;



Environment *get_default_environment(void);

Environment *duplicate_env (Environment *e);

struct Var *id_lookup (Environment const *e, Identifier id);

void add_id (Environment *e, Identifier id, struct Var const *v);
void change_value (Environment *e, Identifier id, struct Var const *new);

void free_env (Environment *e);


#endif

