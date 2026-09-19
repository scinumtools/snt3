% SciNumTools DIPH5
% Environment HDF5 Format Specification
% Version 2.0

# Scope

DIPH5 is the HDF5 interchange format used by SciNumTools to persist an
*evaluated DIPL environment*. It stores evaluated value nodes together with
their DIPL paths, collection structure, units, settings, and provenance.

The persisted object is therefore a data environment suitable for exchange
with scientific applications. It is not a serialization of the complete
parser or evaluation runtime.

This document specifies the on-disk representation. It does not define the
DIPL language itself; the DIPL specification defines the language concepts
represented here.

# Persisted environment boundary

The file contains the evaluated value-node tree and the node-level information
needed to interpret and trace those values. In particular, node settings,
units, schemas, source information, and provenance are persisted as dataset
attributes.

Implementation-specific runtime registries are not serialized as independent
objects. This includes global function definitions, branching state, and the
complete unit and schema registries. DIPH5 version 2 additionally stores a
source manifest containing source identities, paths, parent relationships, and
content fingerprints. It does not embed complete source text or parsed source
nodes. A loaded DIPH5 file must therefore be treated as an evaluated
environment, not as a source from which the original DIPL program can be
reconstructed exactly.

# File identification and versioning

The HDF5 root object MUST contain the following scalar attributes:

| Attribute | Type | Required value |
| --- | --- | --- |
| `_DIPL_Format` | UTF-8 string | `SciNumTools3 Environment` |
| `_DIPL_Schema_Version` | unsigned integer | `2` |

Readers MUST reject files with a different format identifier or unsupported
schema version. Version 2 readers support both version 1 files, which have no
source manifest, and version 2 files. Future schema revisions MUST preserve
the meaning of existing attributes or increment the schema version.

## File naming

Files SHOULD use the `.diph5` filename extension to make their purpose clear.
The extension is only a naming convention and is not normative. Readers MUST
identify a file from the root `_DIPL_Format` and `_DIPL_Schema_Version`
attributes, regardless of its filename.

# Paths and objects

Each component of a fully qualified DIPL path is represented by an HDF5 path
component. A value node is an HDF5 dataset at its resolved path. Intermediate
groups represent DIPL hierarchy and carry `_DIPL_Kind=group` and
`_DIPL_Path` attributes.

The `_DIPL_Path` attribute stores the corresponding fully qualified DIPL path
and is the authoritative mapping back to DIPL semantics.

## Collections

Collection containers and items are represented by groups. Their meaning is
identified by `_DIPL_Kind`:

| `_DIPL_Kind` | Meaning | Additional attributes |
| --- | --- | --- |
| `map` | keyed collection | — |
| `map_item` | item of a keyed collection | `_DIPL_Key` |
| `list` | ordered collection | — |
| `list_item` | item of an ordered collection | `_DIPL_Index` |

Collection item groups may contain nested groups and value datasets.

The complete object-kind table is:

| HDF5 object | `_DIPL_Kind` | `_DIPL_Path` | Other required attributes |
| --- | --- | --- | --- |
| hierarchy group | `group` | group path | — |
| keyed collection | `map` | collection path | — |
| keyed item | `map_item` | item path | `_DIPL_Key` |
| ordered collection | `list` | collection path | — |
| ordered item | `list_item` | item path | `_DIPL_Index` |
| value dataset | `value` | value path | `_DIPL_Node_Type`, `_DIPL_Value_Type` |

An implementation MUST use the kind attribute to distinguish a collection
from an ordinary hierarchy group. HDF5 object names alone are not sufficient,
because a numbered or keyed collection item is deliberately flattened into a
normal group path.

# Value datasets

Every value dataset has `_DIPL_Kind=value`, `_DIPL_Path`,
`_DIPL_Node_Type`, and `_DIPL_Value_Type` attributes. `_DIPL_Array` identifies
array values; scalar values use a scalar dataspace and arrays use a simple
dataspace whose dimensions match the DIPL value shape.

Values are written using native HDF5 numeric types. Strings use fixed-width
UTF-8 HDF5 string datatypes. A null DIPL value is represented by a dataset
with an HDF5 null dataspace.

## Value type table

The `_DIPL_Value_Type` value identifies the logical DIPL type. The following
table lists the concrete types currently written by DIPH5 and their HDF5
storage types:

| `_DIPL_Value_Type` | Logical value | HDF5 datatype | Scalar/array |
| --- | --- | --- | --- |
| `bool` | Boolean | native unsigned 8-bit integer | both |
| `char` | Character | native signed 8-bit integer | both |
| `int8`, `int16`, `int32`, `int64` | Signed integer | matching native signed integer | both |
| `uint8`, `uint16`, `uint32`, `uint64` | Unsigned integer | matching native unsigned integer | both |
| `float32` | Single precision real | native 32-bit float | both |
| `float64` | Double precision real | native 64-bit float | both |
| `float128` | Extended precision real | native long double | both |
| `str` | UTF-8 string | fixed-width UTF-8 string | both |

For a scalar, the dataset has an HDF5 scalar dataspace. For an array, the
dataset has a simple dataspace and `_DIPL_Array` is non-zero. The dimensions
are stored in the dataspace rather than in a separate attribute.

# Node settings and provenance

When present, node settings are stored as attributes on the value dataset:

`_DIPL_Constant`, `_DIPL_Condition`, `_DIPL_Format_Spec`, `_DIPL_Tags`,
`_DIPL_Options`, `_DIPL_Option_Units`, `_DIPL_Schemas`, and
`_DIPL_Value_Origin`.

Source and provenance information uses `_DIPL_Source`, `_DIPL_Source_Line`,
`_DIPL_Source_Code`, and the metadata attributes `description`, `authors`,
`title`, `journal`, `year`, `volume`, `issue`, `pages`, `doi`, `url`,
`version`, `created`, `modified`, and `license`. Physical units are stored in
the `units` attribute using their canonical PUEL representation.

## Source manifest

Version 2 files contain a root `/_DIPL_Sources` group. Each child is a
numbered source-manifest entry so DIPL source names never need to be encoded as
HDF5 object names. Each entry has the following attributes:

| Attribute | Type | Meaning |
| --- | --- | --- |
| `_DIPL_Source_Name` | UTF-8 string | Named DIPL source identifier. |
| `_DIPL_Source_Path` | UTF-8 string | Path recorded when the source was parsed; it may be empty for inline input. |
| `_DIPL_Source_Parent` | UTF-8 string | Parent source identifier, when one exists. |
| `_DIPL_Source_Parent_Line` | unsigned integer | Parent source line associated with the registration. |
| `_DIPL_Source_Hash_Algorithm` | UTF-8 string | Digest algorithm, currently `SHA-256`. |
| `_DIPL_Source_Hash` | UTF-8 string | Lowercase hexadecimal digest of the exact UTF-8 bytes parsed by DIP. |

The manifest is an integrity and provenance aid, not a source archive. A
consumer can hash an available source file and compare the result with the
stored digest, but loading DIPH5 does not recreate executable source
definitions or enable source-qualified lookups. `_DIPL_Sources` is reserved at
the DIPH5 root; a DIPL environment with that top-level path cannot be saved as
a version 2 file.

# Examples

## Scalar values and hierarchy

This DIPL fragment:

```DIPL
simulation
  steps int = 100
  enabled bool = true
```

is represented as:

```text
/simulation                         group   (_DIPL_Kind=group)
/simulation/steps                   dataset (_DIPL_Kind=value,
                                             _DIPL_Path=simulation.steps,
                                             _DIPL_Value_Type=int32)
/simulation/enabled                 dataset (_DIPL_Kind=value,
                                             _DIPL_Path=simulation.enabled,
                                             _DIPL_Value_Type=bool)
```

`/simulation/steps` has a scalar dataspace and a native signed 32-bit HDF5
datatype. `/simulation/enabled` has a scalar dataspace and a native unsigned
8-bit HDF5 datatype.

## Arrays and units

This DIPL value:

```DIPL
boundary[inlet]
  velocity float[3] = [1.0, 0.0, 0.0] m/s
```

is represented as:

```text
/boundary                            group   (_DIPL_Kind=map,
                                             _DIPL_Path=boundary)
/boundary/inlet                      group   (_DIPL_Kind=map_item,
                                             _DIPL_Path=boundary[inlet],
                                             _DIPL_Key=inlet)
/boundary/inlet/velocity             dataset (_DIPL_Kind=value,
                                             _DIPL_Path=boundary[inlet].velocity,
                                             _DIPL_Value_Type=float64,
                                             _DIPL_Array=1,
                                             units=m*s-1)
```

The velocity dataset has rank one and dimension `3`.

## Ordered collections

This DIPL fragment:

```DIPL
samples[]
  time float = 0 s
samples[]
  time float = 1 s
```

is represented by a `list` group containing `list_item` groups:

```text
/samples                            group   (_DIPL_Kind=list,
                                             _DIPL_Path=samples)
/samples/0                          group   (_DIPL_Kind=list_item,
                                             _DIPL_Path=samples[0],
                                             _DIPL_Index=0)
/samples/0/time                     dataset (_DIPL_Path=samples[0].time)
/samples/1                          group   (_DIPL_Kind=list_item,
                                             _DIPL_Path=samples[1],
                                             _DIPL_Index=1)
/samples/1/time                     dataset (_DIPL_Path=samples[1].time)
```

The `_DIPL_Index` attribute, rather than HDF5 group iteration order, defines
the order of list items.

# Compatibility

Implementations SHOULD preserve unknown attributes when editing a DIPH5 file
and MUST ignore attributes they do not understand. They MUST reject a file
whose format identifier or schema version is unsupported rather than silently
interpreting it as a different format.
