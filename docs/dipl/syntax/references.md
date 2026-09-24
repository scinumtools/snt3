# References

References have two main applications.
One can either [import](references.md#imports) some already parsed DIPL nodes into a new location, or [inject](references.md#injections) other node values or contents of text files into a new node.
Besides the two cases, references are also used in [conditions](conditions.md#conditions) and [condition properties](properties.md#condition) that are explained in a separate chapter.
		      
## Reference Notation

A DIPL reference identifies either content from a source or a node within a domain. 
A reference consists of an optional source identifier and an optional path, separated by `?`. 
The `.` symbol has different meanings depending on its position within a reference: 
within a path, it separates hierarchical node names; 
at the beginning of a relative reference, it determines the reference level; 
at the end of a path, it selects the complete subtree rooted at the target node rather than the node itself; 
and alone, `{.}`, it denotes a self-reference to the current node.

### Source and absolute references

The following forms are absolute references:

| Reference                  | Meaning                                       |
|----------------------------|-----------------------------------------------|
| `{source}`                 | Raw content of `source`                       |
| `{?path}`                  | Value of the node `path` in the local domain  |
| `{?path.}`                 | All descendants of `path` in the local domain |
| `{?}` / `{?.}`             | Complete source node tree in the local domain |
| `{source?path}`            | Value of the node `path` in `source`          |
| `{source?path.}`           | All descendants of `path` in `source`         |
| `{source?}` / `{source?.}` | Complete source node tree in `source`         |

The `?` separates the source identifier from the node path. If the source component is omitted, the reference targets the **local domain**, consisting of the nodes parsed from the current DIPL file.

A path without a trailing `.` MUST resolve to a single node. A path ending in `.` selects the complete subtree rooted at the target node, including all descendant nodes recursively, and therefore produces a node set. A path MUST resolve to an existing node; otherwise evaluation MUST fail. A node set used where a single value is required MUST also cause evaluation to fail.
The trailing `.` MAY be omitted when the entire source node tree is imported (`{?}`, `{source?}`). In these cases, the omitted trailing `.` is semantically equivalent to the `?` symbol.

A reference of the form {source} has no path component and returns the raw content of the referenced source as a string. 
Node-based access is provided only when a path component is specified. 
A referenced DIPL source is processed as a separate remote domain.

Absolute references can therefore be used both to **import nodes** and to **inject values**:

```DIPL
driver
  {?child}                       # single local node
  {garage?worker}                # single remote node

passengers
  {?family.}                     # local subtree
  {tourists?family.}             # remote subtree

sites
  {?.}                           # all local nodes
  {london?.}                     # all remote nodes 
  # can be used also without trailing dot: {?} and {london?}

corpus str = {book}              # raw source content
chapter str = {?introduction}    # local node value
citation str = {law?paragraph}   # remote node value
```

The expected result type MUST be compatible with the context in which the reference is used.

### Relative references

A relative reference resolves its path against a node in the current node hierarchy:

```text
{.<path>}
{..<path>}
{...<path>}
...
{.<path>.}
{..<path>.}
{...<path>.}
...
```

The number of leading dots determines the reference level:

* `{.<path>}` resolves `path` relative to the current node;
* `{..<path>}` resolves `path` relative to the parent;
* `{...<path>}` resolves `path` relative to the grandparent;
* in general, **N** leading dots resolve the path relative to the node **N − 1** levels above the current node.

As with absolute references, a trailing `.` selects the complete descendant subtree rather than a single node.

A relative reference MUST NOT traverse beyond the root node. Such a reference is invalid and MUST result in an error.

```DIPL
uncle str = "John"
aunt str = "Jessica"
father str = "William"
  daughter str = {..aunt}
  son str = "Eliah"
    grandson str = {...uncle}
brother-in-law str = {.father}
```

Here `{..aunt}` resolves `aunt` from the parent context of `daughter`, while `{...uncle}` resolves `uncle` from the corresponding ancestor context of `grandson`. `{.father}` resolves `father` relative to the hierarchy containing `brother-in-law`.

### Self-reference

`{.}` is a special self-reference and is distinct from `{.<path>}`. It refers to the **fully evaluated value of the current node** and is valid only within condition properties.

Before `{.}` is evaluated, the current node MUST have undergone:

1. resolution of all references;
2. evaluation of all expressions; and
3. normalization to the canonical unit representation defined by PUEL.

The resulting value retains its complete type information, including dimensionality and unit.

If evaluation of the current node fails, including because of an unresolved reference, type error, or unit incompatibility, `{.}` is undefined and evaluation of the containing condition MUST result in an error.

```DIPL
speed int = 78 kph
  !condition ({.} < 80 kph)
```

`{.}` MUST NOT be used outside condition properties.

## Source Declarations

Sources are named using the `$source` declarator:

```DIPL
$source name = "path/to/file.dip"
```

A source declaration associates a source identifier with a file path. The source may subsequently be referenced any number of times using the absolute reference forms described above.

Sources declared inside a DIPL file MUST resolve their paths relative to the location of that DIPL file.

Implementations SHOULD additionally support defining sources through the host environment (code interface). Source paths supplied through the code interface MUST be interpreted relative to the calling context.

For example:

```DIPL
$source garage = "vehicles/garage.dip"
$source tourists = "people/tourists.dip"
$source book = "texts/book.txt"

driver
  {garage?worker}

passengers
  {tourists?family.}

corpus str = {book}
```

Thus, `$source` declarations establish the mapping between source identifiers and external files, while reference notation determines whether the reference accesses raw source content, a node value, a node subtree, or the evaluated value of a node in the current hierarchy.

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
  {?icecream.scoops.}       # select all children nodes
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

bag {pantry?.}                 # import all nodes
bowl 
  {pantry?fruits}              # selecting a specific node
  {pantry?veggies.potato}      # selecting a specific subnode
plate {pantry?veggies.}        # selecting all subnodes   
```

So far, we have shown how to import regular nodes from a local or remote source.
It is, however, also possible to import sources and custom [units](units.md#units) in the similar way.
The request can select either one ``{<source>?<path>}`` or all ``{<source>?.}`` sources/units.

> [!NOTE]
> Request path is in this case not a node path but name of a source/unit.

Importing sources/units enables users to dynamically modify numerical code units and setting scripts via their DIPL.

``` DIPL
$source init = "initial/settings.dip"
$source {init?.}           # all sources of 'init' are imported
$unit {units?.}            # all units are imported from an imported source 'units'
weight float = 23 mass     # using imported unit
```

## Injections

Injections provide values to node definitions and modifications. They do not insert complete nodes or node sets.

### Value Injection

A valid value injection MUST resolve to either:

* a single node, referenced without a trailing `.`;
* raw source content, referenced without a path.

When a node is referenced, its value is injected into the receiving expression and is subject to the type, dimensionality, and unit rules of that expression.

```DIPL
pop float = 5 km
foo float = 34 cm
bar float = {?foo} m    # resulting value is 0.34 m
baz float = {?foo}      # dimension mismatch
foo = {?pop}            # modification results in 500000 cm
```

Values can also be injected from remote DIPL sources:

```DIPL
$source pressure = "pressures.dip"
pressure float = {pressure?magnetic}
```

For injections, a path always identifies the node whose **value** is to be injected. Unlike node imports, injection paths MUST NOT select a node set.

### Array and String Slicing

Array values can be injected directly or sliced to match the required dimensions of the receiving node. Strings and arrays use the same slicing notation as Python.

A single array element can be selected:

```DIPL
sizes float[3] = [34,23.34,1e34] cm
mysize float = {?sizes}[1]
```

A range of elements can be selected:

```DIPL
masses float[2,2] = [[34,23.34],[1,1e34]] cm
mymass float[2] = {?masses}[:,1]
```

Strings can be sliced in the same way:

```DIPL
person str = "Will Smith"
surname str = {?person}[5:]
```

The resulting values are:

```text
sizes = [3.400e+01, 2.334e+01, 1.000e+34]
mysize = 2.334e+01
masses = [[34,23.34],[1,1e34]]
mymass = [23.34,1e34]
person = "Will Smith"
surname = "Smith"
```

### Raw Source Injection

Raw source injection provides a convenient way to keep large text or data blocks in external files. This separates data from the DIPL structure and makes both the source and the data easier to maintain.

When a reference contains no `?` path component, the referenced source is injected as text rather than as a node list.

```DIPL
$source velocity = "velocity.txt"
$source outputs = "outputs.txt"
$source message = "message.txt"

velocity int[3,4] = {velocity} km/s   # import an array
outputs table = {outputs}             # import a table
message str = {message}               # import text
```

The receiving node is responsible for interpreting the injected text according to its declared type.

### Source and Unit Declaration Injection

Values used by `$source` and `$unit` declarations can themselves be injected from nodes. In these declarations, the injected value supplies the declaration argument rather than the value of an ordinary node.

For `$source`, the referenced node MUST contain a string. For `$unit`, the referenced node MUST contain a floating-point or integer value.

```DIPL
refs str = "path/to/sources.dip"
$source refs = {?refs}

mass float = 1 kg
$unit mass = {?mass}
```

In both cases, the path in the injection refers to the **value of a node**. Injection paths therefore differ from node imports: injections always request a node value and cannot request a node set.
