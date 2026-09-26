DIPfile projects
================

A ``DIPfile`` gathers the units, named sources, reusable schemas, DIPL files,
and inline DIPL definitions used by one parameter environment. It is itself
ordinary DIPL: the project loader supplies the schemas for its four lists and
parses the file with the standard DIP parser.

Project format
--------------

Each project contains only ``units[]``, ``sources[]``, ``schemas[]``, and
``code[]`` items. A schema item has a ``name`` and exactly one of ``file`` or
``string``. Its file or string is a schema body without a ``$schema`` wrapper.
Schema-level ``?`` metadata may appear before the first body node.
Schemas are registered before ``code[]`` is parsed; ``code[]`` items retain
their declared order. Each code item also has exactly one of ``file`` or
``string``.

.. code-block:: dipl

   units[]
     name = "length"
     unit = "23*au"

   sources[]
     name = "constants"
     filepath = "data/constants.dip"

   schemas[]
     name = "settings"
     file = "schemas/settings.dipl"

   schemas[]
     name = "options"
     string = "enabled bool = true"

   code[]
     file = "parameters/base.dip"

   code[]
     string = """
   physics : settings
   output : options
   derived int = ({?answer} + 1)
   """

Paths in ``filepath`` and ``file`` are interpreted relative to the directory
containing the DIPfile. The contents of a ``string`` block are passed to DIP
unchanged, so a root DIPL node inside the block must begin at column zero.
The ``schemas[]`` entries match host-code ``add_schema_file()`` and
``add_schema_string()``; ``code[]`` may still contain ordinary ``$schema``
declarations.

The project manifest is an input description rather than part of the final
parameter environment. It cannot be combined with individual ``--input``
arguments or a DIPH5 ``--load`` command.

Using projects
--------------

The project-loading call and command syntax are documented with their
respective interfaces: :doc:`C++ basic usage <basic-usage>`,
:doc:`the command-line interface <../../integrations/cli>`,
:doc:`the Python binding <../../integrations/python>`, :doc:`the C bindings
<../../integrations/c>`, and the command-oriented :doc:`C++ API
<../../api/dip>`.
