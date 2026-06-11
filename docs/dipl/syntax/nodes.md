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

Node **names** are formed from letters, numbers, underscores, hyphens and dots.
Dots have a special function, because they separate parent and child node names.

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

### 3.1.5. Groups

Nodes can be additionally arranged using special type of node called **group** node.

``` DIPL-Schema   
# Group node schema

<indent><name> 
```

Group nodes do not carry any value, nor do they declare any parameter for further use.
Their function is to group multiple child nodes into a logical structure and their name enters the final node path.

After parsing of nodes, their names are transformed into a path that consists of the original name plus all parent names in the hierarchy separated by a **dot**.
The original node name can already be a path and will be parsed accordingly.

``` DIPL
family                          # group is not parsed
  father str = "Peter"          
    son str = "Benjamin"        # normal notation
  father.daughter str = "Lucia" # using both normal and path notation
family.aunt.dog str = "Lassie"  # using only path notation
```

The example above will result in the following fully-qualified path notation:

``` DIPL
family.father = "Peter"
family.father.son = "Benjamin"
family.father.daughter = "Lucia"
family.aunt.dog = "Lassie"
```

### 3.1.6. Collections

Collections extend group nodes to represent associative and sequential data structures.

A collection is declared by appending square brackets to a group node name:

- `name[key]` denotes an item in a **keyed collection**.
- `name[]` denotes an item in a **indexed collection**. Item indices are assigned implicitly.

A collection path shall be used consistently. Once established as keyed or indexed, all subsequent items on the same path shall use the same collection type.
Collections may be nested.

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

basket.food[fruits].berries[].name str = "strawberry"
basket.food[fruits].berries[].weight float = 300 g

basket.food[fruits].berries[].name str = "cherry"
basket.food[fruits].berries[].weight float = 500 g
```

Or, equivalent JSON representation:

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

Collection items are modified in the same manner as regular nodes.
For indexed collections, the item index shall be specified explicitly.

```DIPL
basket.food[fruits].apples = 3
basket.food[fruits].berries[1].weight = 250 g
```

### 3.1.7. Schemas

Collections may define a **schema** that specifies nodes common to all collection items. A schema acts as a template from which subsequent collection items inherit their structure.

Schemas use the same notation as collections, but replace the collection key or index with an asterisk (`*`).

- `name[*]` defines a schema for a collection.
- Nodes declared or defined within the schema are inherited by all subsequently appended collection items.
- A schema shall be declared before the first collection item is defined.
- Schemas may be used with both keyed and indexed collections.

```DIPL
car[*]
  manufacturer str;
  weight float = 0 kg;
  max_speed float = 0 km/h;

car[civic]
  manufacturer = "Honda"
  weight = 1350 kg
  max_speed = 200 km/h

car[320i]
  manufacturer = "BMW"
  weight = 1570 kg
  max_speed = 235 km/h

car[xc90]
  manufacturer = "Volvo"
  weight = 2100 kg
  max_speed = 180 km/h
```
