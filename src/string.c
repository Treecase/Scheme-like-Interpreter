/*
 * String functions
 *
 */

#include "data.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>



/* _strndup: same as the standard `strndup', but using GC_malloc */
char *_strndup (char const *str, size_t n)
{
    char *out = GC_malloc (n);
    if (out == NULL)
    {   fatal ("Out of memory!");
    }

    for (size_t i = 0; i < n; ++i)
    {   out[i] = str[i];
    }
    return out;
}

/* mkstring: create a string from a char list */
String mkstring (char const *str)
{
    String s = NULL_STRING;

    if (str != NULL)
    {
        s.len   = strlen (str);
        s.size  = s.len + 1;
        s.chars = _strndup (str, s.len);
    }
    return s;
}

/* mknstring: create a string of length n */
String mknstring (char const *str, size_t n)
{
    String s = NULL_STRING;

    if (str != NULL)
    {
        s.chars = _strndup (str, n);
        s.len   = n;
        s.size  = s.len + 1;
    }
    return s;
}

/* stringdup: duplicate a string */
String stringdup (String s)
{   return mkstring (s.chars);
}

/* stringcmp: compare two strings */
int stringcmp (String a, String b)
{
    if (a.chars != NULL && b.chars != NULL)
    {
        size_t n = (a.len > b.len)? a.len : b.len;

        return strncmp (a.chars, b.chars, n);
    }
    else
    {   if (a.chars == b.chars)
        {   return 0;
        }
        else if (a.chars == NULL)
        {   return -1;
        }
        else
        {   return 1;
        }
    }
}

