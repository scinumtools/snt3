# Overrides

Overrides tune the values of an existing parameter model. They do not define
nodes or alter declarations, schemas, properties, or the declared hierarchy.
Dependencies and conditional definitions are evaluated using the replacement
values, so the evaluated model may contain different active nodes.

## Declaration

An override region begins with the `$override` directive at indentation zero.
Its body shall contain one or more value modifications, indented two spaces:

```DIPL
$override
  simulation.resolution = 1024
  simulation.box_size = 200 Mpc

simulation
  resolution int = 512
  box_size float = 100 Mpc
```

Each expanded target shall be a fully qualified path to a value node normally declared
outside override regions. Schema members and collection members may be targeted
using their instantiated paths, such as `materials[copper].density` or
`particles[0].mass`. An override shall not append collection items, select a
subtree, or replace an individual array element or slice. An array replacement
assigns the whole value and shall satisfy its declared dimensions.

Override regions shall not appear inside schemas, groups, collection items,
conditional blocks, or other override regions. Their bodies shall not contain
type declarations, properties, metadata, imports, or collection declarations. Blank lines
and comments are permitted. An empty region is invalid.

For example, `resolution = 1024` is a modification, whereas
`resolution int = 1024` is a definition and is invalid inside an override region.

## Nested Paths

A body may combine dotted paths and indentation, using the same path composition
rules as ordinary DIPL nodes:

```DIPL
simulation
  steps int = 100
  box_size float = 1 Mpc

$override
  simulation
    steps = 1024
    box_size = 200 Mpc
```

Inside an override body, a bare path such as `simulation` is only a prefix for
its children. It shall not create a group or instantiate a schema. The example
registers the targets `simulation.steps` and `simulation.box_size`.

Children shall be indented two spaces beyond their parent. A prefix shall
contain at least one modification, directly or through further prefixes.
Explicit collection keys and indices are allowed as prefixes, for example
`materials[copper]`; empty selectors such as `materials[]` are forbidden.
A value modification may also have nested modifications to existing children
of that value-bearing node. Its own value and each child's value are separate
override targets.

Duplicate detection shall use expanded paths. Thus `simulation.steps = 1024`
and a nested `steps = 1024` under `simulation` conflict even if their values
are identical. Source locations shall retain the original, unexpanded lines.

## Collection and Evaluation

All override regions shall be collected before normal node evaluation.
Multiple regions may appear among top-level inputs; their textual position
shall not establish precedence or change which declarations they target.

A target may be overridden only once. Repeated target paths shall fail even
when their replacement values are identical or originate from different input
files, regions, or host registrations.

When the target declaration is processed, the replacement shall be evaluated
instead of its original value. The original declaration shall remain
syntactically valid, but its original value expression or function shall not
be evaluated. Subsequent ordinary modifications shall not replace an
overridden value. Their structural role and declaration compatibility checks
remain in effect. An override may replace a value marked `!constant` without
removing that property.

Every collected override shall match an instantiated value node. After normal
node processing, any unmatched target shall cause an unresolved override
error. Overrides shall not create missing targets.

## Replacement Values and Units

Replacement values may use the normal value forms supported by the target:
literals, arrays, `none`, references, expressions, and registered value
functions. For example:

```DIPL
base float = 10 cm
radius float = 1 cm
diameter float = ({?radius} * 2) cm

$override
  radius = ({?base} * 2) cm
```

This produces `radius = 20 cm` and `diameter = 40 cm`. Alternative replacements
include `radius = {?base} cm` or `radius = compute_radius() cm`, where the host
has registered the value function `compute_radius`.

Dependencies of a replacement shall be available when the target declaration
is evaluated. Moving the override region shall not enable forward references
or make the target's original value available to its own replacement.

The target's declared type, dimension constraints, units, and properties shall
be preserved. Explicit compatible replacement units shall be converted into
the target's units; omitted units use the target's declared units. Incompatible
physical dimensions shall fail. Units shall not be added to a dimensionless
target. The effective value shall satisfy the normal schema, options,
condition, and format constraints of the target.

## Conditional Definitions

Overrides participate in normal dependency and conditional evaluation:

```DIPL
enabled bool = false
@if ({?enabled})
  steps int = 100
@end

$override
  enabled = true
  steps = 200
```

The overridden `enabled` activates the existing definition of `steps`, whose
value becomes `200`. If `enabled` were overridden to `false` instead, the
override of `steps` would be unresolved and evaluation would fail.

Changing which existing conditional definitions are active does not change
the declared model. Overrides shall not introduce structural definitions or
bypass the conditions under which their targets exist.

## Host Inputs and Provenance

The reference implementation exposes `add_override_string(body)` and
`add_override_file(path)`. Both accept an unwrapped override body containing
modifications and optional nested path prefixes. The first body level starts
at indentation zero. They use the same target, duplicate, and
validation rules as `$override` regions. Registration is atomic: a rejected
body registers no entries. Target existence is checked during model evaluation.

An overridden node exposes its override state. Provenance retains both its
original declaration and the replacement source, including the file path when
available. Constraint diagnostics identify the offending override and include
the declaration as context. DIPH5 snapshots preserve the evaluated value and
override provenance; they do not re-evaluate override expressions on loading.

Override targets belong to the current evaluated model. This directive does
not define remote or source-domain override semantics.
