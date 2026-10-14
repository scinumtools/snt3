# C binding

The C binding provides opaque handles over the public SNT C++ classes. Include
``snt/c/puq.h`` for PUQ operations and ``snt/c/dip.h`` for DIPL parsing.
The interface supports expression evaluation, unit conversion, formatted
output, explicit destruction, and C-compatible error reporting.

Override bodies can be registered with `snt_dip_parser_add_override_string`.
`snt_dip_parser_add_override_file` reads such a body from a file and retains
its source path. `snt_dip_parser_is_overridden` reads the effective flag
after parsing or loading DIPH5. All three use the usual status/error contract.

## Typed DIPL reads

After parsing or loading an environment, `snt_dip_environment_get_node`
returns an owned, read-only node handle. Free every node handle with
`snt_dip_node_free` before freeing its parser. Node handles store a path and
resolve it against the parser's current environment.

Use `snt_dip_node_get_kind` and `snt_dip_node_get_size` to inspect the tree.
Groups expose named children, maps expose sorted keys and keyed items, and lists
expose indexed elements. `snt_dip_node_get_type` and the typed getters read
primitive values and flattened array elements. `snt_dip_node_get_value_size`
returns the scalar/array element count; typed `_at` getters read an element by
zero-based index. Array dimensions are available through `snt_dip_node_get_shape`.

```c
snt_dip_node *node = NULL;
snt_dip_error error = {0};
if (snt_dip_environment_get_node(dip, "simulation.steps", &node, &error) == 0) {
    int64_t steps = 0;
    if (snt_dip_node_get_int64(node, &steps, &error) == 0) {
        /* use steps */
    }
    snt_dip_node_free(node);
}
```

String, path, and metadata getters accept a caller-owned buffer. Pass `NULL`
and capacity zero to query the required byte count, including the terminating
NUL, then call again with a sufficiently large buffer. Strings are UTF-8.
Metadata access returns effective metadata on the requested node, including
explicit groups, collections, items, and values, plus metadata inherited by
schema members. Scalar metadata is accessed by DIPL name
(such as `rationale`); list metadata includes `native`, `requires`, `conflicts`,
`implies`, `see`, and `example`. Use `snt_dip_node_has_metadata` to distinguish
unset metadata from an explicitly empty value.

`snt_dip_node_get_provenance` copies the value's source name, line, and source
code, plus override source information when present. The typed API is an
incremental C ABI surface; it does not yet expose schema definitions.

`snt_dip_node_has_units` and `snt_dip_node_get_units` expose the node's
formatted unit expression. `snt_dip_node_is_overridden` reports whether an
explicit override supplied its effective value. Nodes with `!options` expose
their option count and indexed formatted values, labels, and unit expressions.
