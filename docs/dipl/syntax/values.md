# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.3. Values

### 3.3.1. Empty values

Similarly, as in Python, DIPL parameters can also be set empty using a special keyword ``none``.
Operations with empty parameters conform to basic Python rules and may lead to corresponding warnings, or errors.
DIPL parameters with ``none`` values are considered to be fully defined, rather than only declared.

### 3.3.2. Scalars

Nodes with a single boolean, number or string value are called scalar nodes.
If a node is scalar at its definition, it can be modified only with one value, or as empty.
Example of scalar node values were already shown in section about [data types](datatypes.md#standard-data-types).

### 3.3.3. Arrays

Nodes can also store multiple values in arrays.
Dimensionality of such arrays have to be specified next to the type using bracket notation.

**Dimensionality of arrays**
| Example,        | Description                                                |
|-----------------|------------------------------------------------------------|
| ``[:]``         | Array can have arbitrary number of values		       |
| ``[3:]``        | Array must have minimum of 3 values			       |
| ``[:5]``        | Array must have maximum of 5 values			       |
| ``[1:4]``       | Array must have between 1 and 4 values		       |
| ``[6:,:8,2:7]`` | Settings of individual dimensions are separated by comma   |

Parsing of array values is handled by a custom parser that extracts individual values and array shape.
Final values of nodes are automatically validated according to defined conditions.

``` DIPL
data1 bool[4]   = [true,false,false,true]  # exactly four values
data2 int[:]    = [0,1,2,3,4,5,6]          # any number of values
data3 float[3:] = [0,1.34,1.34e4]          # at least 3 values
data4 float[:4] = [0,1.34,1.34e4]          # maximum of 4 values
data4 str[3:4]  = ["John","Peter","Simon"] # between 3 and 4 values
```

Multiple values above are written in a tight notation without empty spaces in between the values.
Loose notation, with empty spaces, is also permitted.
Additionally, arrays can be parsed from string values, wrapped into double quotes.

``` DIPL
data1 bool[4]   = [true, false, false, true]
data2 int[4]    = "[1, 2, 3, 4]"
data3 str[3:4]  = "[\"John\", \"Peter\", \"Simon\"]"
```

Multidimensional arrays are defined similarly using nested bracket notation.

``` DIPL
matrix int[2,3] = [[0,1,2],[3,4,5]]
```

Node units apply to all values in an array.

``` DIPL
mass float[2:,:2] = [[25,50],[34.2,95.1],[1e3,1e4]] kg
```

### 3.3.4. Blocks

If node values are large or span over several lines, it is possible to use block notation.
Block notation wraps values into triple quote marks, similarly as in Python.
For numerical data types, units can be set after the end of a block.

``` DIPL
# velocity field
velocity int[4,4] = """
[[ 0, 1, 2, 4],
 [ 5, 6, 7, 8],
 [ 9,10,11,12],
 [13,14,15,16]]
""" km/s   # units are optional for numerical data types

# large text
text str = """
Lorem ipsum dolor sit amet, consectetur adipiscing elit, 
sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. 
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris 
nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in 
reprehenderit in voluptate velit esse cillum dolore eu fugiat 
nulla pariatur. Excepteur sint occaecat cupidatat non proident, 
sunt in culpa qui officia deserunt mollit anim id est laborum.
"""
```

### 3.3.5. Tables

When working with large amounts of data, presenting the information in a table can be more concise and easier to read.
For this purpose, a dedicated node type called ``table`` is provided.
This data type parses DIPL nodes from tabulated datasets that are supplied as a block value.

The table structure closely resembles a standard CSV format but uses a specialized header syntax.
The table header contains node declarations corresponding to each column in the table. Each declaration must begin on a new line and must not be indented.
The table data section is separated from the header by a line containing three minus signs (``---``). Within the table body, individual values are separated by a single space.

``` DIPL
output table = """
snapshot int
time float s
intensity float W/m2
---
0 0.234 2.34
1 1.355 9.4
2 2.535 3.4
3 3.255 2.3
4 4.455 23.4
"""
```

Table notation above is equivalent to:

``` DIPL
output
  snapshot int[5] = [0,1,2,3,4]
  time float[5] = [0.234,1.355,2.535,3.255,4.455]
  intensity float[5] = [2.34,9.4,3.4,2.3,23.4] W/m2
```
