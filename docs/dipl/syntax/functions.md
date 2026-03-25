# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.7. Functions

In addition to the expressions defined in the preceding sections, DIPL allows invoking functions provided by the host (interpreter) language. These functions may be used to compute and supply parameter values dynamically.

This mechanism enables users to extend beyond the expressive limits of the built-in syntax and implement custom logic where necessary. In particular, it supports the integration of domain-specific algorithms and specialized functionality commonly required in scientific and technical applications. As such, function calls provide a controlled way to fine-tune DIPL behavior and parameterization.

Function calls follow a standard notation, where the function name is immediately followed by parentheses.

``` DIPL-Schema
# Function schema

<function>()
```

An example of functions used in DIPL code is below.

``` DIPL
side float = 5 cm
volume float = fn_volume() cm3
surface int = fn_surface() mm2
prime bool = is_prime()
value str = print_value()
```

Function names correspond to functions defined by the interpreter language (e.g. C++, Python, ...). 
Each function invocation receives an implicit argument data, which contains a copy of already parsed value nodes. 
If arguments are provided in the function call, they are evaluated and passed in addition to data.

Each function must return a value that corresponds to the node type defined in DIPL. 
This may be either a native host-language type that can be mapped to a DIPL type, or an explicitly constructed DIPL value. 
If the returned value is not compatible with the declared type, evaluation shall fail.

If a function cannot be resolved, or if its execution results in an error, evaluation shall fail. 
Implementations should provide diagnostic information indicating the function name and cause of failure.

Function calls are evaluated during parse-time, as specified by the implementation. 
Unless explicitly stated otherwise, function evaluation should be deterministic.
