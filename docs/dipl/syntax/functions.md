# Functions

In addition to the expressions defined in the preceding sections, DIPL allows invoking functions provided by the host (interpreter) language. These functions may be used to compute and supply parameter values dynamically.

This mechanism enables users to extend beyond the expressive limits of the built-in syntax and implement custom logic where necessary. In particular, it supports the integration of domain-specific algorithms and specialized functionality commonly required in scientific and technical applications. As such, function calls provide a controlled way to fine-tune DIPL behavior and parameterization.

Function calls follow a standard notation, where the function name is immediately followed by parentheses.

An example of functions used in DIPL code is below.

``` DIPL
# standard node definition
side float = 5 cm
# definition of node value from functions
volume float = fn_volume() cm3
surface int = fn_surface() mm2
prime bool = is_prime()
value str = print_value()
# importing nodes from a function into a group
parameters fetch_params()
shape
  shape_settings()
```

Function names correspond to functions provided by the interpreter implementation (e.g., C++, Python, or another supported language).

Each function invocation receives an implicit `data` argument containing a copy of the value nodes that have already been parsed. 
In the reference implementation, this corresponds to a `dip::Environment` object.

If explicit arguments are provided in the function call, they are evaluated and passed to the function in addition to `data`.

A function invoked as a **value injection** shall return a value corresponding to the node type declared in DIPL. 
In the reference implementation, such a return value is represented by `dip::ValueNodeData`. 
A function invoked as a **node import** shall return a list of nodes. 
In the reference implementation, this is represented by `dip::ValueNode::ListType`.

If the returned value is incompatible with the declared DIPL type, evaluation shall fail.
Evaluation shall also fail if the function cannot be resolved or if its execution results in an error. 
Implementations should provide diagnostic information identifying the function name and the cause of the failure.

Function calls are evaluated during parsing, as specified by the implementation.
Unless explicitly stated otherwise, function evaluation should be deterministic.

