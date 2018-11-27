
An interpreter for a Scheme-like language.  
  
  
Currently supports:  
 * Number, String, and Boolean literals
 * Identifiers (including vertical bar notation)
 * Primitive math operations (ie + - / \*)  
 * `define`
 * `lambda`
 * `if`
 * `set!`


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

Conditionals:  
 \> (if #f 'true 'false)  
 : (false)  
 \> (if #t 'true 'false)  
 : (true)  
  
