
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
  
  
  
*Note:* References to page numbers within the source code (eg "/\* TODO: support dotted pair notation **(pg 41)** \*/") refer to page numbers of [r7rs.pdf](https://bitbucket.org/cowan/r7rs/raw/4c27517de187142ad2cf4bcd8cb9199ae1e48c09/rnrs/r7rs.pdf)

