
An interpreter for a Scheme-like language.  
  
  
Currently supports:  
 * Primitive math operations (ie + - / \*)  
 * `define` for variables
 * `lambda` and `define` with functions


For example, to square 10, you might do:  
 \> (\* 10 10)  
 : (100)  

And to define a variable:  
 \> (define a 10)  
 : (\<undefined\>)  
 \> (\* a a)  
 : (100)  

Defining a function:  
 \> (define x (lambda (a b) (\* a b)))  
 : (\<undefined\>)  
 \> (x 10 10)  
 : (100)  

