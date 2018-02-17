
An interpreter for a Scheme-like language.  
  
  
Currently supports:  
    * Primitive math operations (ie + - / * )  
    * Variable assignment with `define`  

As of yet, function assignment is not implemented.



For example, to square 10, you might do:  
 `> (* 10 10)`  
 `return 100`
  
Or, using a variable:  
 `> (define x 10)`  
 `> (* x x)`  
 `return 100`

You can also assign a variable to a primitive function:  
 `> (define x *)`  
 `> (x 10 10)`  
 `return 100`  

Variables can be redefined, too:  
 `> (define x 10)`  
 `> (define x (+ x 5))`  
 `> (x)`  
 `return 15`  

