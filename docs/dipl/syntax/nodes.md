# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.1. Nodes

The DIPL language distinguishes three different ways to create a parameter, depending on which parts of the parameter node are specified.
A parameter can be fully [defined](nodes.md#definition), meaning that all node components—name, type, shape, value, and unit—are provided.
Alternatively, a parameter can be [declared](nodes.md#declaration) by specifying only its name, type, and shape.
After a parameter has been either defined or declared, its value component can be [modified](nodes.md#modification).
Additionally, nodes can be ordered into a [hierarchical](#hierarchy) structure by using indents.

### 3.1.1. Definition

``` DIPL-Schema
# Node definition schema

<indent><name> <type> = <value> <unit>  # <comment>
<indent><name> <type> = <value>         # <comment>
```

All members of definition are separated with at least one empty space, and their order is not interchangeable.

**Indentation** is at the beginning of a line and it determines node [hierarchy](nodes.md#hierarchy).
Every new indentation level should be separated by two empty spaces.

Node **names** are formed from letters, numbers, underscores and hyphens.
Dots have a special function in names, because they separate atomic (parent and child) node names.

There are four main **data types** in DIPL that are based on data types used in Python language: boolean (``bool``), integer (``int``), float (``float``) and string (``str``).

Node **values** are separated from the left-hand members by an equal sign.
Unquoted values consist only of non-whitespace characters.
If a value contains spaces, it must be enclosed in single or double quotes.
If a node value spans multiple lines, a [block](values.md#blocks) must be used.
DIPL parameter values are described in detail in a [separate section](values.md).

The two data types that support **units** are integers and floats.
Units are written directly after a node value and are separated with an empty space.
More detailed description of units is given in a chapter about [units](units.md).
In this subsection, we only describe basic syntax with respect to the nodes.

**Comments** are always at the end of lines and start with a hash sign.
It is also possible to use comments on empty lines to describe the code.

### 3.1.2. Modification

``` DIPL-Schema
# Node modification schema
          
<indent><name> <type> = <value> <unit> 
<indent><name> <type> = <value> 
<indent><name> = <value> <unit> 
<indent><name> = <value> 
```

The first occurrence of a node is called a definition.
All subsequent occurrences of a node with the same name are called modifications.
The last modification is the one used in the parameter output.
The data type of node needs to be set in each definition; however, it can be omitted in subsequent modifications.
If a node is a dimensional parameter, units have to be set in a definition.
One can use different units of the same dimension in modifications; however, the final value of the node will always be converted into units set by the definition.

``` DIPL
size float = 70 cm    # definition
size float = 80 cm    # modifying only value
size = 90 cm          # omitting datatype
size = 100            # omitting units
size = 1 pc           # using different units of length

# Final parameter value is:
# size = 3.086e18 cm  
```

### 3.1.3. Declaration

``` DIPL-Schema	     
# Node declaration schema

<indent><name> <type> <unit> 
<indent><name> <type> 
```

Declaration in DIPL is a special case of definition where equal sign and value part is not specified.
The value must be later set in subsequent DIPL code using modifications, otherwise code will not be valid.

``` DIPL
weight float kg  # declaration
weight = 88      # modification
```

### 3.1.4. Hierarchy

DIPL nodes are organized in a hierarchical way using indentation, i.e. number of empty spaces before nodes.
**Parent** nodes have lower indentation as their **children** nodes.
**Siblings** are nodes which share a common parent and indentation level.
Multiple levels of hierarchy are also allowed, and there can be empty lines between the nodes.
The number of empty spaces for each indentation level can vary, as long as indentation of all children nodes is consistent.

#### 3.1.4.1 Value nodes

``` DIPL
grandfather str = "John"   # parent of Peter and Cintia
  father str = "Peter"     # John's child, Cintia's sibling
                           # Ben's and Lucia's parent
    son str = "Benjamin"   # Peter's child
    daughter str = "Lucia" # Peter's child
  aunt str = "Cintia"      # John's child, Peter's sibling
```

Both parent and children nodes can be either definitions, modifications or declarations.
Nodes in the above example are equivalent to the fully-qualified **path notation** given below.

``` DIPL
grandfather str = "John"  
grandfather.father str = "Peter"     
grandfather.father.son str = "Benjamin"   
grandfather.father.daughter str = "Lucia" 
grandfather.aunt str = "Cintia"  
```
#### 3.1.4.2 Container nodes

Besides basic value nodes, DIPL supports three types of container nodes.

| Structure | Example    | Semantics                    | C++ Equivalent | Python Equivalent |
|-----------|------------|------------------------------|----------------|-------------------|
| Group     | `foo`      | Named children               | `struct`       | `@dataclass`      |
| Map       | `foo[key]` | Children addressed by key    | `std::map`     | `dict`            |
| List      | `foo[]`    | Children addressed by index  | `std::vector`  | `list`            |

Unlike value nodes, container nodes do not directly hold values.
Instead, they serve as organizational structures for child nodes.
Depending on the container type, child nodes are addressed by name, key, or index.

**Groups**

Nodes may be organized into logical hierarchies using a special container node called a **group**.

```DIPL-Schema
# Group node schema

<indent><name>
```

Unlike value nodes, group nodes do not directly store values. Their sole purpose is to organize child nodes into a logical structure. The group's name becomes part of the fully-qualified path of all descendant nodes.

During parsing, node names are expanded into fully-qualified paths by prepending the names of all parent groups, separated by dots (`.`).

Node names may themselves contain dot-separated path segments. Both hierarchical notation and explicit path notation are equivalent and may be freely combined.

```DIPL
family                          # group node
  father str = "Peter"
    son str = "Benjamin"        # hierarchical notation

  father.daughter str = "Lucia" # mixed notation

family.aunt.dog str = "Lassie"  # explicit path notation
```

The example above produces the following fully-qualified node paths:

```DIPL
family.father = "Peter"
family.father.son = "Benjamin"
family.father.daughter = "Lucia"
family.aunt.dog = "Lassie"
```

**Collections**

Collections extend group nodes to represent associative and sequential data structures.

A collection item is declared by appending square brackets to a node name:

- `name[key]` denotes an item in a **keyed collection** (**map**).
- `name[]` denotes an item in an **indexed collection** (**list**). Item indices are assigned implicitly, starting at `0`.

A collection path shall be used consistently. Once a collection has been established as keyed or indexed, all subsequent items on the same path shall use the same collection type.

Collections may be nested.

Collection selectors are interpreted according to the type of the referenced collection.

When resolving a fully-qualified path, all parent nodes shall already exist. Since the container type of each parent node is therefore known, selectors of the form `[value]` can be unambiguously interpreted either as map keys or list indices.

Only the final path segment may create a new node or collection item.

```DIPL
basket                      # group

  food[vegetables]          # keyed collection
    carrots float = 0.5 kg
    potatoes float = 2 kg

  food[fruits]
    apples int = 3
    water_melons int = 1

    berries[]               # indexed collection
      name str = "strawberry"
      weight float = 300 g

    berries[]
      name str = "cherry"
      weight float = 500 g
```

Equivalent fully-qualified path notation:

```DIPL
basket.food[vegetables].carrots float = 0.5 kg
basket.food[vegetables].potatoes float = 2 kg

basket.food[fruits].apples int = 3
basket.food[fruits].water_melons int = 1

basket.food[fruits].berries[0].name str = "strawberry"
basket.food[fruits].berries[0].weight float = 300 g

basket.food[fruits].berries[1].name str = "cherry"
basket.food[fruits].berries[1].weight float = 500 g
```

Equivalent JSON representation:

```json
{
  "basket": {
    "food": {
      "vegetables": {
        "carrots": 0.5,
        "potatoes": 2.0
      },
      "fruits": {
        "apples": 3,
        "water_melons": 1,
        "berries": [
          {
            "name": "strawberry",
            "weight": 300
          },
          {
            "name": "cherry",
            "weight": 500
          }
        ]
      }
    }
  }
}
```

Collection items are accessed and modified in the same manner as regular nodes. For indexed collections, the item index shall be specified explicitly.

```DIPL
basket.food[fruits].apples = 3
basket.food[fruits].berries[1].weight = 250 g
```
