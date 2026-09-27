# C binding

The C binding provides opaque handles over the public SNT C++ classes. Include
``snt/c/puq.h`` for PUQ operations and ``snt/c/dip.h`` for DIPL parsing.
The interface supports expression evaluation, unit conversion, formatted
output, explicit destruction, and C-compatible error reporting.

Override bodies can be registered with `snt_dip_parser_add_override_string`.
`snt_dip_parser_add_override_file` reads such a body from a file and retains
its source path. `snt_dip_parser_is_overridden` reads the effective flag
after parsing or loading DIPH5. All three use the usual status/error contract.
