
An interpreter for a Scheme-like language.  
  
  
Currently supports:
 * Number, String, Symbol, and Boolean literals
 * Identifiers (including vertical bar notation)
 * Primitive math operations (ie `+ - / *`)
 * `define`
 * `lambda`
 * `if`
 * `set!`


For example, to square 10, you might do:  
```Scheme
> (* 10 10)
: 100
```

And to define a variable:  
```Scheme
> (define a 10)
: <undefined>
> (* a a)
: 100
```

Defining a function:  
```Scheme
> (define x (lambda (a b) (* a b)))
: <undefined>
> (x 10 10)
: 100
```

Conditionals:  
```Scheme
> (if #f 'true 'false)
: 'false
> (if #t 'true 'false)
: 'true
```

