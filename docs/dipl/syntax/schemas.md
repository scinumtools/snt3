# DIPL - Language Specification

« Back to [specification](../specification.md#language-syntax)

## 3.10 Schemas

Schemas provide a mechanism for defining reusable node structures.

A schema describes a set of nodes that may later be instantiated within groups, maps, or lists. By allowing common structures to be declared once and reused multiple times, schemas reduce duplication and promote consistency across related data.

Conceptually, a schema is similar to a type definition in a programming language. 
It defines the structure and properties that its instances shall contain, but does not itself create any nodes within the document hierarchy.

### 3.10.1 Declaration and Instantiation

Schemas are declared using the `$schema` directive:

```DIPL-Schema
$schema <name>
  <node declarations>
```

The schema name defines a reusable type. 
Nodes declared within the schema become members of that type and are inherited by all schema instances.

For example, the following schema defines the common structure of a car:

```DIPL
$schema car
  manufacturer str;
  weight float kg;
  max_speed float km/h;

$schema vehicle
  registration str;
  year int;
```

A schema is instantiated by appending one or more schema names after a colon (`:`) to a group, map item, or list item declaration. Multiple schema names are separated by commas:

```DIPL-Schema
<node> : <schema-name>[, <schema-name>...]
```

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

### 3.10.2 Nested Schemas

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

### 3.10.3 Rules

The following rules shall apply:

* Schema names shall be unique within a document.
* A schema shall be declared before it is instantiated.
* A schema declaration does not create any nodes in the document hierarchy.
* A schema may contain value nodes and container nodes.
* A schema may instantiate other schemas.
* Schemas may be instantiated within groups, map items, and list items.
* The structure defined by a schema is inherited by all of its instances.
