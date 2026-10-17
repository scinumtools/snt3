# Schemas

Schemas provide a mechanism for defining reusable node structures.

A schema describes a set of nodes that may later be instantiated within groups, maps, or lists. By allowing common structures to be declared once and reused multiple times, schemas reduce duplication and promote consistency across related data.

Conceptually, a schema is similar to a type definition in a programming language. 
It defines the structure and properties that its instances shall contain, but does not itself create any nodes within the document hierarchy.

## Declaration and Instantiation

Schemas are declared using the `$schema` directive.
The schema name defines a reusable type. 
Nodes declared within the schema become members of that type and are inherited by all schema instances.

Metadata properties may immediately follow the schema declaration, before its
first member. They describe the reusable definition and stay in the schema
registry: they are not copied to the group, collection, or item that applies
the schema, nor to its value nodes. Metadata indented beneath a value member
describes that member and is copied when the schema is applied:

```DIPL
$schema settings
  ?descr "Reusable physics settings"
  ?since "0.8.3"
  speed float = 2 m/s
    ?descr "Flow speed"
```

Schema-level descriptive metadata are available through the environment's
schema registry. A schema may also declare `!metadata` constraints before its
first member. These constraints apply to metadata on each group or item that
uses the schema; they are not descriptive metadata and are not inherited by
instances.

For example, the following schemas define common structures that can be combined to describe a car:

```DIPL
$schema car
  manufacturer str;
  weight float kg;
  max_speed float km/h;

$schema vehicle
  registration str;
  year int;
```

A schema is instantiated by appending one or more schema names after a colon (`:`) to a group, map item, or list item declaration. 
Schemas cannot be applied to a value node, including a value node that has child nodes.
Multiple schema names are separated by commas.
For example, the following creates two `car` instances, each of which also conforms to the `vehicle` schema:

```DIPL
car[civic] : car, vehicle
  manufacturer = "Honda"
  weight = 1350 kg
  max_speed = 200 km/h
  registration = "M-AB 1234"
  year = 2020

car[320i] : car, vehicle
  manufacturer = "BMW"
  weight = 1570 kg
  max_speed = 235 km/h
  registration = "M-CD 5678"
  year = 2022
```

Schemas are independent of container type and may therefore be instantiated within groups, maps, or lists.

For example:

```DIPL
$schema person
  name str;
  age int years;
```

As a group:

```DIPL
father : person
  name = "Peter"
  age = 45 years
```

As a map item:

```DIPL
people[john] : person
  name = "John"
  age = 32 years
```

As a list item:

```DIPL
people[] : person
  name = "Anna"
  age = 28 years
```

Schema fields can be declared constant without a value. Each instance then
supplies its own initial value; later ordinary modifications to that field
are rejected:

```DIPL
$schema edge
  subject str
    !constant
  relation str
    !constant
  object str
    !constant

edges[example] : edge
  subject = "person"
  relation = "has_substantial_form"
  object = "human"

edges[other] : edge
  subject = "animal"
  relation = "is_a"
  object = "organism"

edges[example].relation = "is_a"  # error: already assigned
```

All declared fields still need a value in each instance. A schema field
defined with a value is constant immediately; its instance cannot assign a
different initial value through an ordinary modification. Explicit
``$override`` regions retain their usual ability to replace constants.

## Required metadata

Schemas can require metadata on the instance itself and on individual schema
members. Use `required` to require that an annotation is present, or `nonempty`
to require a value containing non-whitespace content:

```DIPL
$schema citation
  !metadata authors required
  locator str
    ?url "https://example.org/reference"
    !metadata url nonempty

records map : citation
records[example]
  ?authors "Example author"
  locator = "Section 1"
```

The schema-level rule applies to the metadata on each applying group or item.
The member rule applies to `locator` metadata. The schema's URL annotation is
a default, so it satisfies `nonempty` when an instance leaves that field
unspecified. An explicit empty instance annotation replaces the default and
fails `nonempty`. Under `required`, an explicitly empty annotation counts as
present.

`nonempty` rejects empty and whitespace-only scalar strings. For list-valued
metadata, the list must contain an item and every item must contain
non-whitespace text. Thus an empty list or a list containing an empty item
fails. Metadata property names use their regular spelling without the `?`,
such as `authors`, `url`, or `requires`.

Constraints are evaluated after schema defaults and instance annotations have
been combined, after declarations are complete, and after supported overrides
have been applied. An explicit empty override does not fall back to a schema
default. Requirements from multiple applied schemas are combined: `nonempty`
also satisfies `required`, while both rules remain independently visible in
inspection. Unknown metadata names and rules are rejected during parsing.
Diagnostics identify the affected path, metadata property, rule, and schema
source location.

The schema hierarchy inspection API exposes instance-level and member-level
constraints. DIPH5 snapshots retain evaluated metadata and schema application
provenance, but do not contain reusable schema definitions. A snapshot records
the result of evaluation; loading it does not re-run the original schema
constraints.

## Collection schemas

Schemas can also be applied to entire `map` and `list` collections. 
In this case, the schema is specified once in the collection declaration and is automatically applied to all subsequent items.

```DIPL
$schema vehicle
  speed float = 212 kph
  weight float = 1320 kg

$schema road
  tires str = "Continental"

vehicles map : vehicle       # schema applies to all items
vehicles[car] : road         # schema applies to this specific item
vehicles[ship]
```

In this example, both `car` and `ship` inherit the nodes defined by the `vehicle` schema, because it is applied to the entire `vehicles` collection. 
The `car` item additionally inherits the nodes defined by the `road` schema, while `ship` does not.

## Nested Schemas

Schemas may contain both value nodes and container nodes, including nested groups, maps, lists, and other schema instances.

```DIPL
$schema address
  street str;
  city str;
  country str;

$schema person
  name str;
  age int years;

  residence : address
```

Because schemas describe structure independently of container type, they provide a uniform mechanism for defining reusable data models throughout a DIPL document.

## Rules

The following rules shall apply:

* Schema names shall be unique within a document.
* A schema shall be declared before it is instantiated.
* A schema declaration does not create any nodes in the document hierarchy.
* A schema may contain value nodes and container nodes.
* A schema may instantiate other schemas.
* Schemas may be instantiated within groups, map items, and list items.
* The structure defined by a schema is inherited by all of its instances.
* Metadata remains optional unless an instance or member constraint is declared.
* Schema metadata constraints are validated against effective metadata after
  defaults, instance annotations, and supported overrides have been applied.
