# References

References have two main applications.
One can either [import](references.md#3.4.2-imports) some already parsed DIPL nodes into a new location, or [inject](references.md#3.4.3.-injections) other node values or contents of text files into a new node.
Besides the two cases, references are also used in [conditions](conditions.md#3.9.-conditions) and [condition properties](properties.md#3.8.2.-condition) that are explained in a separate chapter.
		      
## Node Referencing

Node referencing is a core feature of DIPL that enables the creation of reusable code structures. 
A reference consists of a source identifier and an optional path component separated by ``?`` (i.e., ``{<source>?<path>}``).

The ``source`` identifier refers to a named source, which maps to a file path. 
The ``path`` component specifies a node path within the referenced domain.

Sources are defined once using the ``$source`` declaration and may be referenced multiple times throughout the DIPL document.

``` DIPL
$source name = "path/to/file.dip"
```

All implementations of the DIPL language SHOULD support defining sources through the host environment (code interface).
- Source paths defined via the code interface MUST be interpreted relative to the calling context.
- Source paths defined within DIPL files MUST be interpreted relative to the location of the respective DIPL file.

### Domains

Depending on the context, sources may refer to:
- Text files, when no path is provided (raw content access), or
- DIPL files, when a path is provided (node-based access), or when the entire file is referenced.

The **local domain** consists of all nodes parsed within the current DIPL file.
A **remote domain** refers to a separate DIPL file, which is processed independently.

### Local References

If the source component is omitted, the reference implicitly targets the local domain.

```DIPL
size float = 34 cm
human
  height float = {?size}
```

In this example the height of a human is set to the size defined by the preceeding ``size`` node.

### Path Semantics

The path component specifies which nodes are selected from a domain.

A path path MUST resolve to either:
- a single node, or
- a set of nodes (when using ``*``)

The ``.*`` suffix selects all descendant nodes of the target node recursively, including the entire subtree rooted at that node.

If a path does not exist, evaluation MUST fail.
If a reference returns multiple nodes in a context that requires a single value, evaluation MUST fail.

### Self-Reference and Relative References (`{.}`, `{.<path>}`)

The `{.}` reference is a **self-reference** and is valid only within [condition properties](properties.md#3.8.2.-condition). 
It MUST NOT appear in any other context.

The `{.}` reference evaluates to the **fully evaluated value of the current node**, after:

* resolution of all references;
* evaluation of all expressions; and
* normalization to the canonical unit representation defined in the Units specification.

The resulting value retains its complete type information, including dimensionality and unit.

If evaluation of the current node's value fails for any reason, including unresolved references, type errors, or unit incompatibility, the `{.}` reference is undefined. 
In such a case, evaluation of the condition MUST result in an error.

**Relative references** identify nodes relative to the current node and are written as ``{.<path>}``, ``{..<path>}``, ``{...<path>}``, etc.

The number of leading dots determines the level at which the path is resolved:

* ``{.<path>}`` resolves `path` relative to the **current node**;
* ``{..<path>}`` resolves `path` relative to the **parent** of the current node;
* ``{...<path>}`` resolves `path` relative to the **grandparent** of the current node;
* in general, *N* leading dots resolve `path` relative to the node *N − 1* levels above the current node.

A relative reference MUST NOT traverse beyond the root node. 
Such a reference is invalid and MUST result in an error.

The following example demonstrates the resolution of self and relative references:

```DIPL
uncle str = "John"
aunt str = "Jessica"
father str = "William"
  daughter str = {..aunt}
  son str = "Eliah"
    grandson str = {...uncle}
brother-in-law str = {.father}
```

Here:

* ``{..aunt}`` in `daughter` resolves `aunt` relative to `daughter`'s parent, `father`.
* ``{...uncle}`` in `grandson` resolves `uncle` relative to `grandson`'s grandparent.
* ``{.father}`` in `brother-in-law` resolves `father` relative to the current node's parent/context according to the node hierarchy.

### Reference Result Types

``{<source>}`` returns:
- raw content for text sources
- root node set for DIPL sources

absolute references ``{?<path>}``, ``{<source>?<path>}`` and relative references ``{.<path>}`` return:
- a single node, or
- a node set (when using *)

self-reference ``{.}`` refers exclusively to the fully evaluated value of the current node and is therefore distinct from `{.<path>}`.

The expected type MUST match the usage context. Otherwise, evaluation MUST fail.

``` DIPL
# Reference requests

# Inject source content
corpus str = {book}             # remote

# Inject node value
chapter str = {?introduction}   # local
paragraph str = {law?preamble}  # remote

# Import single node
driver
  {?child}                      # local
  {garage?worker}               # remote

# Import child nodes
passengers
  {?family.*}                   # local
  {tourists?family.*}           # remote

# Import all nodes
sites
  {?*}                          # local
  {london?*}                    # remote

# Self reference
speed int = 78 kph
  !condition ({.} < 80 kph)     # local

# Relative reference
father str = "John"
  son str = {.father}           # local
```

## Imports

Imports can be used to insert referenced nodes directly into the current DIPL hierarchy.
Name paths of imported nodes are embedded into the current node hierarchy as shown in the following examples.


``` DIPL
icecream 
  waffle str = "standard"
  scoops
    strawberry int = 1
    chocolate int = 2

bowl
  {?icecream.scoops.*}      # select all children nodes
plate {?icecream.waffle}    # select a specific node
```

The node hierarchy above is equivalent to the one given below.

``` DIPL
icecream.waffle = "standard"
icecream.scoops.strawberry = 1
icecream.scoops.chocolate = 2
bowl.strawberry = 1
bowl.chocolate = 2
plate.waffle = "standard"
```

The example above demonstrates importing local nodes; the same mechanism applies to external DIPL files.
In this case, a source name must be specified before the question mark.

``` DIPL
$source pantry = "pantry.dip"

bag {pantry?*}                 # import all nodes
bowl 
  {pantry?fruits}              # selecting a specific node
  {pantry?veggies.potato}      # selecting a specific subnode
plate {pantry?veggies.*}       # selecting all subnodes   
```

So far, we have shown how to import regular nodes from a local or remote source.
It is, however, also possible to import sources and custom [units](units.md#36-units) in the similar way.
The request can select either one ``{<source>?<path>}`` or all ``{<source>?*}`` sources/units.

> [!NOTE]
> Request path is in this case not a node path but name of a source/unit.

Importing sources/units enables users to dynamically modify numerical code units and setting scripts via their DIPL.

``` DIPL
$source init = "initial/settings.dip"
$source {init?*}           # all sources of 'init' are imported
$unit {units?*}            # all units are imported from an imported source 'units'
weight float = 23 [mass]   # using imported unit
```

## Injections

Injections do not insert whole nodes.
They are used in node definitions and modifications instead of values.

A valid injection can reference only a single node or a text content of a file.

``` DIPL
size1 float = 34 cm       # standard definition
size2 float = {?size1} m  # definition using import with other units
size3 float = {?size2}    # definition using import with same units
size1 = {?size2}          # modifying by import

# Nodes above will have the following values:
#
# size1 = 3400 cm
# size2 = 34 m
# size3 = 34 m
```

It is also possible to inject values from remote DIPL files:

``` DIPL
$source pressure = "pressures.dip"
pressure float = {pressure?magnetic}
```
   
Arrays can be imported either directly or can be sliced to match dimensions of a host node.
Slicing of arrays and also strings adopts the same notation as used in Python.
An example of sliced injected arrays is below:

``` DIPL
person str = "Will Smith"
surname str = {?person}[5:]   # slicing a string

# selecting a single value
sizes float[3] = [34,23.34,1e34] cm      
mysize float = {?sizes}[1]  

# selecting range of values
masses float[2,2] = [[34,23.34],[1,1e34]] cm    
mymass float[2] = {?masses}[:,1]              

# Above nodes will have values:
#
# person = "Will Smith"
# surname = "Smith"
# sizes = [3.400e+01, 2.334e+01, 1.000e+34]
# mysize = [2.334e+01]
# masses = [[34,23.34],[1,1e34]]
# mymass = [23.34,1e34]
```

Value injection can also be used to keep large text blocks in external files.
This makes both the code and text data more readable and easily editable.
Note that when requests do not include a question mark with a path, DIPL imports files as a text and not as a node list.

``` DIPL   
$source velocity = "velocity.txt"
$source outputs = "outputs.txt"
$source message = "message.txt"

velocity int[3,4] = {velocity} km/s   # import an array
outputs table = {outputs}             # import a table
message str = {message}               # import a text
```

Values of source and unit definitions can also be injected from other nodes.

> [!NOTE]
> In comparison to imports, request path in injections is always path of a node.

This adds an additional scalability to the code.
Referenced nodes by sources have to be strings and referenced nodes by units have to be floats or integers.

``` DIPL
refs str = "path/to/sources.dip"  # node named 'refs'
$source refs = {?refs}            # source named 'refs'

mass float = 1 kg                 # node named 'mass'
$unit mass = {?mass}              # unit named 'mass'
```
