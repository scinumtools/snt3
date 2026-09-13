# Functions

DIPL allows functions provided by the host implementation to be invoked from within a DIPL document.
Functions extend the built-in expression syntax with custom logic and allow parameter values or nodes to be generated dynamically.

**Note:** The input and output data of a user-defined function may differ depending on the host implementation and the particular use case. However, the function interface defined by DIPL remains consistent: the environment list is passed to the function in the defined form, and the function returns a value/node list in the defined form.

## Function Calls

A function call consists of a function name followed by parentheses:

```DIPL 
# standard node definition
side float = 5 cm

# value injection from functions
volume float = fn_volume() cm3
surface int = fn_surface() mm2
prime bool = is_prime()
value str = print_value()

# node import from functions
parameters fetch_params()
shape
  shape_settings()
```

The function name identifies a function provided by the interpreter implementation. Implementations may provide functions written in the host language, such as C++, Python, or another supported language.

## Function Arguments

Every function invocation receives an implicit `data` argument containing a copy of the value nodes that have already been parsed at the point of invocation.

In the reference implementation, this argument is represented by a `dip::Environment` object.

Explicit arguments may additionally be supplied in the function call. These arguments are evaluated before the function is invoked and are passed to the function together with the implicit `data` argument.

```DIPL 
scale float = 10 cm
result float = calculate({?scale}, 2)
```

The exact representation and calling convention of the implicit `data` argument are implementation-defined.

## Function Results

A function can be used either as a **value injection** or as a **node import**.

When used as a value injection, the function MUST return a value compatible with the declared DIPL node type. In the reference implementation, this is represented by `dip::ValueNodeData`.

When used as a node import, the function MUST return a list of nodes. In the reference implementation, this is represented by `dip::ValueNode::ListType`.

The function result MUST therefore match the syntactic context in which the function is invoked. An incompatible result MUST cause evaluation to fail.

## Evaluation and Errors

Function calls are evaluated during parsing, as specified by the implementation.

Evaluation MUST fail if:

* the function cannot be resolved;
* the function execution results in an error; or
* the returned value is incompatible with the expected DIPL type or result context.

Implementations SHOULD provide diagnostic information identifying the function and the cause of the failure.

Unless explicitly specified otherwise by the function or implementation, function evaluation SHOULD be deterministic.
