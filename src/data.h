/*
 * Data handling stuff
 *
 */


#ifndef _DATA_H
#define _DATA_H



// error logging
#define error(...)  fprintf (stderr, "Error: ");\
                    fprintf (stderr, __VA_ARGS__)
#define fatal(...)  error (__VA_ARGS__);\
                    exit (-1)


/* function datatype */
struct func {
    char          *name;
    unsigned long argc;
    char          *base;
    int           baselen;
};

/* generic data struct */
typedef struct {
    unsigned char type;
    union {
        float         fval;
        int           ival;
        char          *str;
        struct func   func;
        unsigned long vari; // variable table offset
    } value;
} Data;

// datatypes
enum data_types {
    T_FUNC,
    T_VARIABLE,
    T_INTEGER,
    T_FLOAT,
    T_STRING,
    T_ERROR = (unsigned char)-1,
};

/* s-expression struct */
typedef struct {
    Data *data;
    int  len;
} S_Exp;


#endif

