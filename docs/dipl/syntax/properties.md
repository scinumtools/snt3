# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.8. Properties

Numerical codes usually require initial parameter values with a specific format.
In this section, we summarize properties that can be used in DIPL to restrict node values and describe their format.
Each node property directive is given on a new line immediately after node definitions, declarations or modification.
All properties must have consistent indent, two whitespaces higher than their parent node.

### 3.8.1. Options

**Used by:** ``int``, ``float``, ``str``

Initial code parameters often accept only a few discrete input values, also called options.
These can be explicitly described during node definition or declaration.

``` DIPL-Schema
# Schema of a node option
<indent>= <value> <unit>
<indent>= <value>
```

Node options can be specified for all data types except boolean.
In case of boolean, the two options (``true`` or ``false``) are implicitly set.
Individual options are specified directly under particular nodes starting with equal sign.
Options cannot be separated from nodes by an empty line.
All options corresponding to the same node must have the same indent, higher than the node.

``` DIPL
coordinates int = 1
  = 1  # carthesian
  = 2  # polar
  = 3  # spherical
```

If the node has an other value than one of the options, DIPL will throw an error message.

Node definition and individual options can have different units, but they must have the same dimensionality. The final value of such modified node will be, however, converted into units specified in the definition.

``` DIPL
animal str = "dog"
  = cat
  = dog
  = horse

energy float = 23 erg   # definition
  = 43                  # option in same units
  = 23 erg              # option in same units
  = 3e-7 J              # option in different unit of energy

energy = 3e-7 J         # modification
# energy = 3 erg        # final value
```

If the number of options is too large, and it is not practical to write each option on a new line, it is also possible to use a shorthand ``!options`` option notation.

``` DIPL-Schema
# Schema of an option array

<indent>!options <value> <unit>
<indent>!options <value>
```

The expected value of this clause is a list of values.
It can be given explicitly or as a [reference](references.md#3.4.-references).
Similiarly as in the cases above, units are optional and must be of the same dimension as those given in the corresponding definition.
   
Several ``!options`` clauses are allowed for the same node (e.g. each with different units).
They combine into a single array of options against which the node value is evaluated.
     
``` DIPL
animal str = "dog"
  !options ["cat","dog","horse"]

energy float = 23 J
  !options [23, 45, 10, 234, 490, 1939] J
  !options [34, 234] erg
# all options: [23, 45, 10, 234, 490, 1939, 3.4e-6, 2.34e-7] J
```

### 3.8.2. Condition

**Used by:** ``int``, ``float``, ``str``, ``bool``

Numerical values can usually have values ranging in some intervals.
To restrict node values to some particular interval, it is possible to set a logical condition using ``!condition`` directive and a logical expression.
A given expression has to be evaluated as ``true`` after each definition or modification of a node.

``` DIPL-Schema
# Schema of a node condition requirement

<indent>!condition ("<expression>")    
<indent>!condition ("""
<expression>
""")                                   
```

In the example below, node ``energy`` can have values in a range of 23 and 26 erg.
The actual value of node ``energy`` is matched using a special [self-reference sign](references.md#3.4.-references) ``{?}``.

``` DIPL
energy float = 25 erg
  !condition ("23 < {?} && {?} < 26")
```

### 3.8.3. Format

**Used by:** ``str``

In general, string values wrapped into quote marks can contain all characters and can be arbitrary long.
This can be restricted by defining their ``!format`` using standard regular expressions.

``` DIPL-Schema
# Schema of a node format requirement

<indent>!format <value>
```

In the following example, node 'name' can contain only small and capital letters:
   
``` DIPL
name str = "Ferdinant"
  !format "[a-zA-Z]+"
```

### 3.8.4. Constants

**Used by:** ``int``, ``float``, ``str``, ``bool``

Sometimes nodes have to stay constant and exclude all possible modifications.
This can be achieved by a directive ``!constant``.

``` DIPL-Schema
# Schema of a constant node requirement
<indent>!constant
```

Node ``name`` in the following example cannot be further modified.
     
``` DIPL
name str = "John"
  !constant
name = "Mary"   # this modification will raise an error exception
```

### 3.8.5. Tags

**Used by:** ``int``, ``float``, ``str``, ``bool``

Tags provide a simple and effective mechanism for organizing and categorizing large sets of parameters.
Any data type that supports tagging may define the dedicated ``!tags`` property, which accepts a list of tag values.
In practice, it is recommended to use string values for tags to ensure consistency and interoperability.

``` DIPL-Schema
# Schema for node tags property

<indent>!tags <value>
```

Once assigned, tags enable nodes to be queried and filtered after parsing through the use of tag selectors.
Every DIPL implementation SHOULD provide support for selecting parsed nodes based on their associated tags.

### 3.8.6. Description

**Used by:** ``int``, ``float``, ``str``, ``bool``

Notes about parameters in DIPL may be written as comments; however, such comments are not included in automatically generated documentation.
For this purpose, a dedicated node property ``!description`` is provided.
The abbreviated form ``!desc`` may be used as an alternative.

``` DIPL-Schema
# Schema for node description

<indent>!description <value>
<indent>!desc <value>
```
