
An interpreter for a Scheme-like language.  
  
  
Currently supports:  
 * Primitive math operations (ie + - / \*)  
 * `define`s for variables


For example, to square 10, you might do:  
    > (* 10 10)  
    : (100)

And to define a variable:
    > (define a 10)
    : (a)
    > (* a a)
    : (100)

