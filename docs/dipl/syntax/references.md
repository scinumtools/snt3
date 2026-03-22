# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.4. References

References have two main applications.
One can either [import](references.md#3.4.2-imports) some already parsed DIPL nodes into a new location, or [inject](references.md#3.4.3.-injections) other node values or contents of text files into a new node.
Besides the two cases, references are also used in [conditions](conditions.md#3.9.-conditions) and [condition properties](properties.md#3.8.2.-condition) that are explained in a separate chapter.
		      
### 3.4.1. Node Referencing

Node referencing is a core feature of DIPL that enables the creation of reusable code structures. 
A reference consists of a source identifier and an optional query component separated by ``?`` (i.e., ``{<source>?<query>}``).

The ``source`` identifier refers to a named source, which maps to a file path. 
The ``query`` component specifies a node path within the referenced domain.

Sources are defined once using the ``$source`` declaration and may be referenced multiple times throughout the DIPL document.

``` DIPL-Schema
# Source Definition Schema

<indent>$source <name> = <path>
```

``<name>`` is the source identifier.  
``<path>`` specifies the location of the referenced file.

All implementations of the DIPL language SHOULD support defining sources through the host environment (code interface).
- Source paths defined via the code interface MUST be interpreted relative to the calling context.
- Source paths defined within DIPL files MUST be interpreted relative to the location of the respective DIPL file.

#### 3.4.1.1. Domains

Depending on the context, sources may refer to:
- Text files, when no query is provided (raw content access), or
- DIPL files, when a query is provided (node-based access), or when the entire file is referenced.

The **local domain** consists of all nodes parsed within the current DIPL file.
A **remote domain** refers to a separate DIPL file, which is processed independently.

#### 3.4.1.2. Local References

If the source component is omitted, the reference implicitly targets the local domain.

```DIPL-Schema
{?<query>}
```

is equivalent to:

```DIPL-Schema
{<local>?<query>}
```

where ```<local>``` denotes the current document.

#### 3.4.1.3. Query Semantics

The query component specifies which nodes are selected from a domain.

A query path MUST resolve to either:
- a single node, or
- a set of nodes (when using ``*``)

The ``.*`` suffix selects all descendant nodes of the target node recursively, including the entire subtree rooted at that node.

If a query path does not exist, evaluation MUST fail.
If a reference returns multiple nodes in a context that requires a single value, evaluation MUST fail.

#### 3.4.1.4. Self-Reference (``{?}``)

The ``{?}`` reference is only valid within [condition properties](properties.md#3.8.2.-condition) and MUST NOT appear in any other context.
It refers to the fully evaluated value of the current node after:
- resolution of all references,
- evaluation of all expressions, and
- normalization to the canonical unit representation defined in the Units specification.

The resulting value retains its full type, including dimensionality and unit.

If evaluation of the node’s value fails for any reason (e.g., unresolved references, type errors, or unit incompatibility), the ``{?}`` reference is undefined and condition evaluation MUST result in an error.

#### 3.4.1.5. Reference Result Types

``{<source>}`` returns:
- raw content for text sources
- root node set for DIPL sources

``{?<query>}`` and ``{<source>?<query>}`` return:
- a single node, or
- a node set (when using *)

The expected type MUST match the usage context. Otherwise, evaluation MUST fail.

``` DIPL-Schema
# Schema of reference requests

# content of a source is returned
{<source>}                     # remote
          
# single node is returned
{?<query>}                     # local
{<source>?<query>}             # remote

# children nodes are returned
{?<query>.*}                   # local
{<source>?<query>.*}           # remote

# all nodes are returned
{?*}                           # local
{<source>?*}                   # remote

# node self-reference
{?}                            # local
```

### 3.4.2 Imports

Imports can be used to insert referenced nodes directly into the current DIPL hierarchy.

``` DIPL-Schema
# Schema of node imports

<indent>{<request>}
<indent><name> {<request>}
```

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
It is, however, also possible to import sources and custom :doc:`units` in the similar way.
The request can select either one ``{<source>?<query>}`` or all ``{<source>?*}`` sources/units.

``` DIPL-Schema
# Schema for importing sources and units

<indent>$source {<request>}
<indent>$unit {<request>}
```

> [!NOTE]
> Request query is in this case not a node path but name of a source/unit.

Importing sources/units enables users to dynamically modify numerical code units and setting scripts via their DIPL.

``` DIPL
$source init = "initial/settings.dip"
$source {init?*}           # all sources of 'init' are imported
$unit {units?*}            # all units are imported from an imported source 'units'
weight float = 23 [mass]   # using imported unit
```

### 3.4.3. Injections

Injections do not insert whole nodes.
They are used in node definitions and modifications instead of values.

``` DIPL-Schema
# Schema of node value injections

<indent><name> <type> = {<request>} <unit>	     
<indent><name> <type> = {<request>}
<indent><name> = {<request>} <unit>	     
<indent><name> = {<request>}
```

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
   
Arrays can be imported either directly or can be sliced to match dimensions of a host node using the following schemas:

``` DIPL-Schema
# Schema of an array slice reference

{?<query>}[<slice>]            # node query from a local domain
{<source>?<query>}[<slice>]    # node query from a remote domain
```

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
Note that when requests do not include a question mark with a query, DIPL imports files as a text and not as a node list.

``` DIPL   
$source velocity = "velocity.txt"
$source outputs = "outputs.txt"
$source message = "message.txt"

velocity int[3,4] = {velocity} km/s   # import an array
outputs table = {outputs}             # import a table
message str = {message}               # import a text
```

Values of source and unit definitions can also be injected from other nodes.

``` DIPL-Schema
# Schema of node value injections

<indent>$unit <name> = {<request>}
<indent>$source <name> = {<request>}
```

> [!NOTE]
> In comparison to imports, request query in injections is always path of a node.

This adds an additional scalability to the code.
Referenced nodes by sources have to be strings and referenced nodes by units have to be floats or integers.

``` DIPL
refs str = "path/to/sources.dip"  # node named 'refs'
$source refs = {?refs}            # source named 'refs'

mass float = 1 kg                 # node named 'mass'
$unit mass = {?mass}              # unit named 'mass'
```
