# Properties

Numerical codes usually require initial parameter values with a specific format.
In this section, we summarize properties that can be used in DIPL to restrict node values and describe their format.
Each node property directive is given on a new line immediately after node definitions, declarations or modification.
All properties must have consistent indent, two whitespaces higher than their parent node.

## Directives

### Options

**Used by:** ``int``, ``float``, ``str``

Initial code parameters often accept only a few discrete input values, also called options.
These can be explicitly described during node definition or declaration.

The expected value of this clause is a list of values.
It can be given explicitly or as a [reference](references.md#references).

Node options can be specified for all data types except boolean.
In case of boolean, the two options (``true`` or ``false``) are implicitly set.
If the node has an other value than one of the options, DIPL will throw an error message.

Node definition and individual options can have different units, but they must have the same dimensionality. 
The final value of such modified node will be, however, converted into units specified in the definition.
Options with explicit units are converted to the node's declared units before
the effective value is compared with them. For an integer node, the effective
value must also satisfy the [integer conversion rule](units.md#integer-values-after-unit-conversion).

``` DIPL
animal str = "dog"
  !options ["cat","dog","horse"]

energy float = 23 erg   # definition
  !options [43, 89]              # option in same units
  !options [23, 34] erg          # option in same units
  !options [3e-7, 5e+3] J        # option in different unit of energy

energy = 3e-7 J         # modification
# energy = 3 erg        # final value
```
   
Multiple ``!options`` clauses are allowed for the same node (e.g. each with different units).
They combine into a single array of options against which the node value is evaluated.

```DIPL
energy float = 23 J
  !options [23, 45, 10, 234, 490, 1939] J
  !options [34, 234] erg
# all options: [23, 45, 10, 234, 490, 1939, 3.4e-6, 2.34e-7] J
```

Optional display labels can be attached to options with one ``!labels`` list.
Its strings correspond, in order, to the combined values from every ``!options``
directive on the node. The number of labels must equal the number of options.
Labels are preserved in DIPH5 and exposed by inspection tools; they do not
change which values are accepted. Nodes without ``!labels`` behave as before.

``` DIPL
resolution int = 16
  !options [16, 32, 64]
  !labels ["Low", "Medium", "High"]
```

For options declared in several clauses, provide all labels in one list:

``` DIPL
energy float = 23 J
  !options [23, 45] J
  !options [34] erg
  !labels ["Reference", "High", "Low"]
```

### Condition

**Used by:** ``int``, ``float``, ``str``, ``bool``

Numerical values can usually have values ranging in some intervals.
To restrict node values to some particular interval, it is possible to set a logical condition using ``!condition`` directive and a logical expression.
A given expression has to be evaluated as ``true`` after each definition or modification of a node.

In the example below, node ``energy`` can have values in a range of 23 and 26 erg.
The actual value of node ``energy`` is matched using a special [self-reference sign](references.md#references) ``{.}``.

``` DIPL
energy float = 25 erg
  !condition (23 < {.} && {.} < 26)
```

### Format

**Used by:** ``str``

In general, string values wrapped into quote marks can contain all characters and can be arbitrary long.
This can be restricted by defining their ``!format`` using standard regular expressions.

In the following example, node 'name' can contain only small and capital letters:
   
``` DIPL
name str = "Ferdinant"
  !format "[a-zA-Z]+"
```

### Constants

**Used by:** ``int``, ``float``, ``str``, ``bool``

Sometimes nodes have to stay constant and exclude ordinary modifications.
This can be achieved by a directive ``!constant``.
If the node is declared without a value, its first assignment supplies the
initial value. Later ordinary modifications are rejected. This applies
independently to each instance of a schema field. An explicit ``$override``
may still replace a constant value.

Node ``name`` in the following example cannot be further modified.
     
``` DIPL
name str = "John"
  !constant
name = "Mary"   # this modification will raise an error exception
```

### Tags

**Used by:** ``int``, ``float``, ``str``, ``bool``

Tags provide a simple and effective mechanism for organizing and categorizing large sets of parameters.
Any data type that supports tagging may define the dedicated ``!tags`` property, which accepts a list of tag values.
In practice, it is recommended to use string values for tags to ensure consistency and interoperability.

Once assigned, tags enable nodes to be queried and filtered after parsing through the use of tag selectors.
Every DIPL implementation SHOULD provide support for selecting parsed nodes based on their associated tags.

Tags should classify nodes for discovery and selection, such as identifying settings
used by an exporter, diagnostic tool, or user interface. Keep tags as simple labels;
they should not encode instructions, expressions, dependencies, or formatting rules.

Structured behavior belongs in typed DIPL nodes and reusable schemas—for example,
an optional child schema defining export units or applicability. Descriptive
information belongs in metadata. Consuming applications select nodes by their tags,
inspect their typed settings and metadata, and implement the relevant behavior.

This separation keeps configuration explicit, supports validation and reuse, and
avoids creating an application-specific language inside tag strings.

``` DIPL
resolution int = 32
  !tags ["box", "grid"]
```

### Delimiter

**Used by:** `table`

When entering values using a `table` node, columns are separated by whitespace by default.

For some input formats, such as CSV or TSV, a different column delimiter is required. DIPL therefore allows additional options to be specified for `table` nodes to define the delimiter used when parsing the table.

This makes it possible, for example, to prepend a DIPL header to a standard CSV file and use it directly as a DIPL table.

```DIPL
output table = """
snapshot int
time float s
intensity float W/m2
---
0, 0.234, 2.34
1, 1.355, 9.4
2, 2.535, 3.4
"""
  !delimiter ","
```

## Metadata

**Used by:** `int`, `float`, `str`, `bool`, and schema definitions

Metadata provide additional information about a parameter without affecting its value or interpretation. They are intended for documentation, provenance, licensing, and versioning purposes, and may be used by documentation generators, scientific workflows, and other tooling.

In particular, provenance metadata describe the origin of a parameter's value, such as the publication, dataset, or other source from which it was obtained. This allows scientific and engineering data to remain traceable and properly attributed.

Unlike comments, metadata are part of the parsed document and are preserved during processing.
Metadata may be attached to any explicit node in a dot-separated hierarchy:
groups, map/list collections, map/list items, and values. For example, a map
item can carry citation metadata directly, while its typed fields continue to
be validated by the map's schema. Metadata is available through node
inspection and is retained in DIPH5 artifacts.

At indentation zero before the first node in a `.dip` source, `?` metadata
describes the source file itself. It is available through source inspection
(`snt dip sources`) and is not inherited by the nodes declared in that file.

On a `$schema` declaration, metadata before the first member describe the
reusable definition. Metadata attached to an individual schema member provide
defaults for instances of that member. Explicit metadata on an instance
replaces the corresponding schema-member field, while fields not set on the
instance retain their defaults. Metadata on a schema definition itself is not
inherited by instances. Explicitly empty metadata values are preserved and can
be distinguished from fields that are absent in inspection results.

Schemas can impose metadata requirements with `!metadata`:

```DIPL
$schema citation
  !metadata authors required
  locator str
    ?url "https://example.org/reference"
    !metadata url nonempty
```

At the schema level, the rule applies to metadata on each group or item that
uses the schema. Beneath a schema member, it applies to that member's metadata.
Write the metadata name without its `?` prefix. `required` checks presence, so
an explicitly empty annotation satisfies it. `nonempty` rejects empty and
whitespace-only scalar strings. For list-valued metadata, it requires at least
one item and rejects empty or whitespace-only items.

Member metadata defaults count toward constraints. An explicit instance value
replaces the matching default, including an explicitly empty value. Constraints
are checked after declarations and supported overrides. Metadata remains
optional for schemas that declare no such rule. Schema inspection exposes the
rules.

Table headers use indentation to distinguish table metadata from column
metadata. Properties at indentation zero before the first column describe the
table node; properties indented by two spaces beneath a column describe that
column. This header syntax is shared by inline block tables and external
`.dipt` files:

```DIPL
measurements table = """?descr "Experiment measurements"
time float s
  ?descr "Elapsed time"
speed float m/s
  ?descr "Measured flow speed"
---
0 3
1 4
"""
```

The table and column properties use the same metadata fields and merge rules
as other DIPL nodes. Table-level metadata can also be attached to the table
declaration in the enclosing `.dip` file.

The following metadata properties are currently defined:

| Property    | Description                                                    |
| ----------- | -------------------------------------------------------------- |
| `?descr`    | Human-readable description of the parameter.                   |
| `?authors`  | Author(s) of the referenced publication or data source.        |
| `?title`    | Title of the referenced publication, report, or dataset.       |
| `?journal`  | Journal, book, conference proceedings, or other publication.   |
| `?year`     | Year of publication or release.                                |
| `?volume`   | Publication volume.                                            |
| `?issue`    | Publication issue.                                             |
| `?pages`    | Page range within the publication.                             |
| `?doi`      | Digital Object Identifier (DOI).                               |
| `?url`      | URL of the referenced resource.                                |
| `?version`  | Version of the referenced document or dataset.                 |
| `?created`  | Creation date of the referenced resource or metadata.          |
| `?modified` | Last modification date of the referenced resource or metadata. |
| `?license`  | License governing the referenced resource or dataset.          |
| `?rationale` | Why the selected or default value is appropriate. |
| `?native` | Native target name(s), such as an application configuration key. |
| `?requires` | FQP or capability required for this node to apply. |
| `?conflicts` | FQP or capability that cannot coexist with this node or value. |
| `?implies` | Setting or capability normally enabled or required by this choice. |
| `?see` | Related FQP cross-reference(s). |
| `?example` | Recommended, optionally unit-aware example value(s). |
| `?recommended_range` | Advisory range; unlike `!condition`, this does not enforce validity. |
| `?performance_impact` | Computational-cost or memory implication. |
| `?scientific_impact` | Modelling or physical consequence. |
| `?deprecated` | Deprecation notice or version. |
| `?replacement` | FQP that supersedes a deprecated node. |
| `?since` | Software or schema version that introduced the node. |
| `?category` | Documentation grouping independent of hierarchy. |
| `?visibility` | Documentation or UI level, such as `basic`, `advanced`, or `expert`. |

`?native`, `?requires`, `?conflicts`, `?implies`, `?see`, and `?example`
accept either one scalar value or an array of strings. All other metadata
properties accept one scalar value.

**Example**

```DIPL
thermal_conductivity float = 401 W/(m*K)
  ?descr "Thermal conductivity of high-purity copper measured at 293.15 K"
  ?authors "John A. Smith, Emily R. Johnson, Michael T. Brown"
  ?title "Thermal Conductivity of High-Purity Copper from 100 K to 500 K"
  ?journal "Journal of Materials Science"
  ?year 2024
  ?volume 59
  ?issue 8
  ?pages "4123-4137"
  ?doi "10.1007/s10853-024-12345-6"
  ?url "https://doi.org/10.1007/s10853-024-12345-6"
  ?version "Published Version"
  ?created "2024-03-18"
  ?modified "2024-04-02"
  ?license "CC BY 4.0"
```
