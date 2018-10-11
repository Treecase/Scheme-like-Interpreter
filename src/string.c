/*
 * String functions
 *
 */

#include "data.h"

#include <stdlib.h>
#include <string.h>



/* mkstring: create a string from a char list */
String mkstring (char const *const str)
{
    String r;

    r.chars = strdup (str);
    r.len   = strlen (str);
    r.size  = r.len + 1;

    return r;
}

/* free_string:  */
void free_string (String s)
{   free (s.chars);
    s.len  = 0;
    s.size = 0;
}

/* stringcmp: compare two strings */
int stringcmp (String a, String b)
{   /* use the length of the shortest string */
    size_t n = (a.len > b.len)? a.len : b.len;
    return strncmp (a.chars, b.chars, n);
}

