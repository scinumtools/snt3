# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.5. Expressions

Node values can also be defined indirectly using expressions.
Expression values consist of multiple values combined with mathematical expressions wrapped in additional parentheses, while they can span on multiple lines. The code will be automatically wrapped into a single line based on the balance of the parentheses

``` DIPL-Schema
# Expression schema
   
# using double quotes
(<expression>)

# using block notation
(<expression>       # first part
 <expression>)      # second part
```

There are 4 different types of expressions in DIPL.
Three of them (logical, numerical and textual, i.e. templates) are described below.
Dimensional expressions, i.e. [units](units.md#3.6.-units), are described in a separate chapter.

> [!NOTE]
> All operators used in expressions must be separated from values with at least one empty space.

Possible input values used by expressions are summarized in the following table:

**Input values**

| Schema     | Name       | Example                       |
|------------|------------|-------------------------------|
| ``<bool>`` | boolean    | ``true``/``false``            |
| ``<num>``  | numerical  | ``24 cm``                     |
| ``<ref>``  | reference  | ``{?energy}``                 |
| ``<expr>`` | expression | ``12 cm == {?width} && true`` |

Operators summarized below are evaluated according to their priority from 1 to 4 and can be nested accordingly.

### 3.5.1. Logical

Logical expressions are used to generate values for boolean nodes.
The expressions can be composed of multiple nested logical operators that always return a true or false value.

An example of a logical expression is given below.
       
``` DIPL
a bool = true
b float = 23.43 cm
c bool = (false || ( {?b} == 23.43 cm || ~{?a} ) 
          && {?a} || ~!{?c})
# Node 'c' will be evaluated as 'true'
```

Expression operators evaluate only boolean values, either given directly or as a result of a nested expression.

**Logical operators**

| Syntax            | Priority | Description                                                                              |
|-------------------|----------|------------------------------------------------------------------------------------------|
| `A \|\| B \|\| C ...` | 4        | Logical OR operator returns true if at least one expression (`A`, `B`, `C`, ...) is true |
| `A && B && C ...` | 3        | Logical AND operator returns true if all expressions (`A`, `B`, `C`, ...) are true       |

**Parentheses**

| Syntax              | Priority | Description                                                                              |
|---------------------|----------|------------------------------------------------------------------------------------------|
| `(<expr>)`        | 2        | Parentheses operator evaluates expression `A` in a separate thread and returns its value |

Numerical values with dimensions compared using comparison operations are automatically converted into same units. The result of such comparison is always a boolean value.

> [!NOTE]
> At the moment, it is possible to compare only scalar numerical values. Support for array comparison is planned to be provided in next versions of DIPL.

**Comparison operators**

| Syntax   | Priority | Description                                                                                                          |
|----------|----------|----------------------------------------------------------------------------------------------------------------------|
| `A == B` | 1        | Equality operator returns true if `A` and `B` have same dimensions and numeraical value up to EQUAL\_PRECISION       |
| `A != B` | 1        | Inequality operator returns true if `A` and `B` do not have same dimension or numerical value up to EQUAL\_PRECISION |
| `A >= B` | 1        | Greater or equal operator returns true if `A` is greater or equal (up to EQUAL\_PRECISION) than `B`                  |
| `A <= B` | 1        | Smaller or equal operator returns true if `A` is smaller or equal (up to EQUAL\_PRECISION) than `B`                  |
| `A > B`  | 1        | Greather than operator returns true if `A` is greater than `B`                                                       |
| `A < B`  | 1        | Smaller than operator returns true if `A` is smaller than `B`                                                        |
  
**Single value operators**

| Syntax    | Priority | Description                                                             |
|-----------|----------|-------------------------------------------------------------------------|
| `~<bool>` | 1        | Negation operator returns true if value `A` is false                    |
| `!<ref>`  | 1        | Definition operator returns true if <reference> node exists             |
| `~!<ref>` | 1        | Non-definition operator returns true if <reference> node does not exist |

### 3.5.2. Numerical

Numerical expressions are used to generate values for numerical node types.
If given, the expression result is automatically converted into node units.
The target units should be placed after the expression parentheses end.

``` DIPL
const
   c float = 299792458 m/s
energy float = (2 kg * pow( {?const.c}, 2 )) eV

# Node 'energy' will be parsed as 1.79751 eV
```

> [!NOTE]
> DIPL does not aim to substitute advanced numerical programming languages.
> Numerical expressions in DIPL are supposed to give a quick tool for generation of input values that can be easily derived from other parameters.
> Therefore, it implements only the most basic mathematical operations on scalar values.
> More advanced operations can be added in the future versions of DIPL.

Operators used in numerical expressions are summarized below:

**Arithmetics**

| Syntax  | Priority | Description                                    |
|---------|----------|------------------------------------------------|
| `A + B` | 3        | Addition of two values of a same dimension     |
| `A - B` | 3        | Substraction of two values of a same dimension |
| `A * B` | 2        | Multiplication of two values                   |
| `A / B` | 2        | Division of two values                         |

Parentheses operators evaluate expressions in a separate thread and return its final value.
Most of the following operators require, that the final value has no dimensions.

**Parentheses operators**

| Syntax                  | Priority | Description                                                                   |
|-------------------------|----------|-------------------------------------------------------------------------------|
| `( <expr> )`            | 1        | Standard parenthesis operator                                                 |
| `exp( <expr> )`         | 1        | Returns exponential value of a dimensionless expression.                      |
| `pow( <expr>, <expr> )` | 1        | Returns first expression risen on a power of second dimensionless expression. |
| `ln( <expr> )`          | 1        | Returns natural logarithmic value of a dimensionless expression.              |
| `log10( <expr> )`       | 1        | Returns common logarithmic value of a dimmensionless expression.              |
| `sin( <expr> )`         | 1        | Returns sine value of a dimensionless expression.                             |
| `cos( <expr> )`         | 1        | Returns cosine value of a dimensionless expression.                           |
| `tan( <expr> )`         | 1        | Returns tangens value of a dimensionless expression.                          |
| `min( <expr>, <expr> )` | 1        | Returns minimum of the two expressions.                                       |
| `max( <expr>, <expr> )` | 1        | Returns maximum of the two expressions.                                       |
| `floor( <expr> )`       | 1        | Returns the greatest integer less than or equal to the expression             |
| `ceil( <expr> )`        | 1        | Returns the smallest integer greater than or equal to the expression          |
| `round( <expr> )`       | 1        | Returns the closest integer or equal to the expression                        |

> [!NOTE]
> Unlike the outer parentheses that define an expression, the inner parentheses must contain at least one whitespace separating the enclosed content. 
> For example, `exp(2 cm)` is invalid, whereas `exp( 2 cm )` is valid.
	   
### 3.5.3. Templates

Templates, or formatted text values, are used to convert node values into a textual representation.
They follow the same syntax as standard string values, but are prefixed with an `f` before the opening quotation mark.
All value types can be embedded into the text using standard Python-style formatting notation.

``` DIPL
id int = 345
name str = "Tina"
body
  weight float = 62.3 kg
  height float = 177 cm
married bool = true

person str = f"""
ID:      {{?id}:05d}
Name:    {{?name}}
Weight:  {{?body.weight}:.3e}
Height:  {{?body.height}:.2f}
Married: {{?married}}
\{not a  reference}
"""

# Node 'person' will be parsed as:
#
# ID:      00345
# Name:    Tina
# Weight:  6.230e+01
# Height:  177.00
# Married: True
```

String expressions interpret starting double curly brackets always as a reference.
Single curly brackets are interpreted as a plain test.

Basic syntax of parsing operators is described below:
    
| Syntax             | Description                          |
|--------------------|--------------------------------------|
| `{<ref>}`          | Default reference of a node value.   |
| `{<ref>:<format>}` | Formatted reference of a node value. |
	
Arrays can also be parsed using templates, however, without specifying a format.
The format depends on the default Python string casting functions:

``` DIPL
name str = "Will Smith"
widths float[2,3] = [[23.4,235.4,34],[1e10,2e23,5e20]]

preview str = f"""
Surname:  {{?name}[5:]}
Scalar:   {{?widths}[1,1]:.2e}
Array:
{{?widths}[:,1:]}
"""

# Node 'preview' will be parsed as:
#
# Surname:  Smith
# Scalar:   2.00e+23
# Array:
# [[2.354e+02 3.400e+01]
#  [2.000e+23 5.000e+20]]
```
