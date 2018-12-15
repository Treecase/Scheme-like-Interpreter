/*
 * LISP Builtin functions
 *
 * ie + - * /, etc.
 *
 *
 * NOTE: MKBUILTIN returns `Var *_builtin_[argument] (size_t argc, Var **argv, Environment env)
 *
 */

#define _NO_UNDEF_MKDEFINE
#include "builtins.h"
#include "data.h"
#include "eval.h"
#include "util.h"

#include <stdlib.h>

#define NOT_A_NUMBER_ERROR(arg)                 (mkerr_var (EC_INVALID_ARG, "%s -- Invalid type. %v is not a Number", __func__, arg))
#define ARG_COUNT_MISMATCH_ERROR(requires, got) (((got) > (requires))? mkerr_var (EC_INVALID_ARG,\
                                                                                  "%s -- Too many arguments. Max %zi, got %zi",\
                                                                                  __func__,\
                                                                                  requires,\
                                                                                  got)\
                                                                     : mkerr_var (EC_INVALID_ARG,\
                                                                                  "%s -- Too few arguments. Min %zi, got %zi",\
                                                                                  __func__,\
                                                                                  requires,\
                                                                                  got))
#define GENERAL_FAILURE_ERROR()                 (mkerr_var (EC_GENERAL, "%s -- ???", __func__))



/* add: given a list (n1 n2 ... nN), returns the sum of these */
MKBUILTIN(add)
{
    if (argv->type != VAR_NIL)
    {
        double sum = 0;

        for (Var *n = argv; n->type != VAR_NIL; n = n->p.cdr)
        {
            Var *num = eval (car (n), env);

            if (num->type == VAR_ATOM && num->a.type == ATM_NUMBER)
            {   sum += num->a.num;
            }
            else
            {   return NOT_A_NUMBER_ERROR(num);
            }
        }
        return var_atom (atm_num (sum));
    }
    return ARG_COUNT_MISMATCH_ERROR(1, 0);
}

/* sub: Subtraction. Can take infinite arguments
 *  NOTE: Also supports negation, ie (- 1) ===> -1  */
MKBUILTIN(sub)
{
    if (argv->type != VAR_NIL)
    {
        Var *first = eval (car (argv), env);
        if (first->type == VAR_ATOM && first->a.type == ATM_NUMBER)
        {
            if (cdr (argv)->type != VAR_NIL)
            {
                double difference = first->a.num;

                for (Var *n = argv->p.cdr; n->type != VAR_NIL; n = n->p.cdr)
                {
                    Var *num = eval (car (n), env);

                    if (num->type == VAR_ATOM && num->a.type == ATM_NUMBER)
                    {   difference -= num->a.num;
                    }
                    else
                    {   return NOT_A_NUMBER_ERROR(num);
                    }
                }
                return var_atom (atm_num (difference));
            }
            else
            {   return var_atom (atm_num (-first->a.num));
            }
        }
        else
        {   return NOT_A_NUMBER_ERROR(first);
        }
    }
    return ARG_COUNT_MISMATCH_ERROR(1, 0);
}

/* mul: Multiplication. Can take infinite arguments */
MKBUILTIN(mul)
{
    if (argv->type != VAR_NIL)
    {
        double product = 1;

        for (Var *n = argv; n->type != VAR_NIL; n = n->p.cdr)
        {
            Var *num = eval (car (n), env);

            if (num->type == VAR_ATOM && num->a.type == ATM_NUMBER)
            {   product *= num->a.num;
            }
            else
            {   return NOT_A_NUMBER_ERROR(num);
            }
        }
        return var_atom (atm_num (product));
    }
    return ARG_COUNT_MISMATCH_ERROR(1, 0);
}

/* div: Division. Can take infinite arguments */
MKBUILTIN(div)
{
    if (argv->type != VAR_NIL)
    {
        Var *first = eval (car (argv), env);

        if (first->type == VAR_ATOM && first->a.type == ATM_NUMBER)
        {
            if (cdr (argv)->type != VAR_NIL)
            {
                double quotient = first->a.num;

                for (Var *n = argv->p.cdr; n->type != VAR_NIL; n = n->p.cdr)
                {
                    Var *num = eval (car (n), env);
                    if (num->type == VAR_ATOM && num->a.type == ATM_NUMBER)
                    {   quotient /= num->a.num;
                    }
                    else
                    {   return NOT_A_NUMBER_ERROR(num);
                    }
                }
                return var_atom (atm_num (quotient));
            }
            else
            {   return var_atom (atm_num (1 / first->a.num));
            }
        }
        else
        {   return NOT_A_NUMBER_ERROR(first);
        }
    }
    return ARG_COUNT_MISMATCH_ERROR(1, 0);
}


/* lambda:  (lambda <formals> <body>)
 *
 * Returns a Function which takes the arguments listed in <formals>
 * and returns the result of evaluating <body> with these arguments */
MKBUILTIN(lambda)
{
    env = env;  /* (shut up about unused variables, gcc!) */

    Var *formals = car (argv);
    Var *body    = car (cdr (argv));

    if (formals->type == VAR_PAIR)
    {
        LISPFunction f;
        f.env  = formals;
        f.body = body;

        _Function fn;
        fn.type = FN_LISPFN;
        fn.fn = f;

        return var_atom (atm_fn (fn));
    }
    else
    {   return mkerr_var (EC_INVALID_ARG,
                          "%s -- Invalid argument list. %v is not a List",
                          __func__,
                          formals);
    }
}

/* define:  (define <identifier> <value>)
 *
 * Defines <identifier> to point to <value> in current environment */
MKBUILTIN(define)
{
    Var *identifier = car (argv);
    Var *value      = eval (car (cdr (argv)), env);

    if (identifier->type == VAR_ATOM && identifier->a.type == ATM_IDENTIFIER)
    {
        /* try `set!' first, so we avoid duplicate variables */
        if (_builtin_set (argv, env)->type != VAR_UNDEFINED)
        {
            /* `set!' failed, so we add `identifier' to the env */
            Var *p = var_pair (identifier, value);
            env->p.cdr = cons (env->p.car, env->p.cdr);
            env->p.car = p;
        }
        return var_undefined();
    }
    else
    {   return mkerr_var (EC_INVALID_ARG,
                          "%s -- Invalid identifier. %v is not an ID",
                          __func__,
                          identifier);
    }
}


/* if:  (if <test> <consequent> <alternate>)
 *      (if <test> <consequent>)
 *
 * Takes 2 or 3 arguments. The first is the test, which will
 * decide which branch to take. The second is the consequent,
 * which will be followed if <test> evaluates to `#f'. The
 * optional third argument is the alternate, which will be
 * followed if <test> DOES NOT evaluate to `#f' */
MKBUILTIN(if)
{
    Var *test       = eval (car (argv), env),
        *consequent = car (cdr (argv)),
        *alternate  = car (cdr (cdr (argv)));

    debug ("test: %v  consequent: %v  alternate: %v",
           test, consequent, alternate);

    if (eq (test, var_false())->a.boolean == true)
    {   return eval (alternate, env);
    }
    else
    {   return eval (consequent, env);
    }
}


/* set!:    (set! <identifier> <expression>)
 *
 * Sets the Var identified by <identifier> to the evaluation of
 * <expression>. */
MKBUILTIN(set)
{
    bool found_id = false;

    Var *id         = car (argv);
    Var *expression = eval (car (cdr (argv)), env);

    if (id->type == VAR_ATOM && id->a.type == ATM_IDENTIFIER)
    {
        for (Var *c = env; c->type != VAR_NIL; c = cdr (c))
        {   if (eq (car (car (c)), id)->a.boolean)
            {
                found_id = true;
                c->p.car->p.cdr = expression;
                break;
            }
        }

        if (found_id)
        {   return var_undefined();
        }
        else
        {   return mkerr_var (EC_INVALID_ARG,
                              "Couldn't find Identifier %v",
                              car (argv));
        }
    }
    else
    {   return mkerr_var (EC_INVALID_ARG,
                          "%s -- Invalid identifier. %v is not an ID",
                          __func__, id);
    }
}


/* include:     (include <string1> <string2> ...) */
/* TODO: implement include */
MKBUILTIN(include)
{
    argv = argv; env = env;
    return mkerr_var (EC_GENERAL, "%s not implemented", __func__);
}



#undef MKBUILTIN
#undef _GETBUILTINNAME

#undef NOT_A_NUMBER_ERROR
#undef ARG_COUNT_MISMATCH_ERROR
#undef GENERAL_FAILURE_ERROR

