# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.7. Functions

In addition to expressions described in previous sections, DIPL code can access functions of its interpreter language and use them to fill parameter values.
This is especially useful when syntax of expressions is not enough to solve some complicated problem.

Function calls are, similarly as expressions, wrapped in parentheses.
Instead of an expression string, the inner argument is simply a function name.

``` DIPL-Schema
# Function schema

(<function>)
```

An example of functions used in DIPL code is below.

``` DIPL
side float = 5 cm
volume float = (fn_volume) cm3
surface int = (fn_surface) mm2
prime bool = (is_prime)
value str = (print_value)
```

Function names in parentheses correspond to function defined by the interpreter language (e.g. C++, Python, ...) and receive an argument ``data`` that holds a copy of already parsed value nodes.
Each function must return a value that correspond to the node type defined in DIPL.
This can be either a native data type or DIPL data type.
	     
