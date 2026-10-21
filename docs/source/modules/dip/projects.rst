DIPfile projects
================

A ``DIPfile`` gathers the units, named sources, reusable schemas, override
files, DIPL files, and inline DIPL definitions used by one parameter
environment. It is itself ordinary DIPL: the project loader supplies the
schemas for its five lists and its optional ``format`` group, then parses the
file with the standard DIP parser.

Project format
--------------

Each project contains ``units[]``, ``sources[]``, ``schemas[]``,
``overrides[]``, and ``code[]`` items, plus an optional ``format`` group for
:doc:`the DIPL developer tools <../../integrations/devtools>`. A schema item
has a ``name`` and exactly one of ``file`` or ``string``. Its file or string is
a schema body without a ``$schema`` wrapper.
Schema-level ``?`` metadata may appear before the first body node.
Each override item has a required ``file`` field. Its file contains value
modifications without a ``$override`` wrapper. Schemas and overrides are
registered before model evaluation, regardless of where their entries appear
in the manifest; ``code[]`` items retain their declared order. Each code item
has exactly one of ``file`` or ``string``.

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

   overrides[]
     file = "parameters/overrides.dip"

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
``add_schema_string()``; ``overrides[]`` matches ``add_override_file()``.
Override targets must exist in the evaluated model and may be specified only
once across all override sources. ``code[]`` may still contain ordinary
``$schema`` declarations. An empty override file is accepted and leaves the
model unchanged.

For example, ``parameters/overrides.dip`` can contain:

.. code-block:: dipl

   output
     enabled = false

The project manifest describes inputs rather than evaluated parameters. A live
environment retains its registrations in ``env.project_entries()`` in DIPfile
order. Each entry records its kind, name, declared value, resolved file path
when applicable, and the DIPfile source and line. These entries are not stored
in a DIPH5 snapshot. The command-line ``--project`` option cannot be
combined with individual ``--input`` arguments or a DIPH5 ``--load`` command.

Using projects
--------------

The project-loading call and command syntax are documented with their
respective interfaces: :doc:`C++ basic usage <basic-usage>`,
:doc:`the command-line interface <../../integrations/cli>`,
:doc:`the Python binding <../../integrations/python>`, :doc:`the C bindings
<../../integrations/c>`, and the command-oriented :ref:`C++ API
<module-api-dip>`.

Static browser bundles
----------------------

``package_project()`` evaluates a DIPfile with the native C++ parser and
collects the text files it opens. The resulting ``ProjectBundle`` stores the
DIPfile and its local sources with relative paths. ``DIP::add_project(bundle)``
then evaluates those bytes with the same registration and table parsing rules,
without filesystem or network reads:

.. code-block:: cpp

   #include <snt/dip/dip.h>
   #include <snt/dip/project_bundle.h>

   auto bundle = snt::dip::package_project("DIPfile");
   snt::dip::DIP parser;
   parser.add_project(bundle);
   auto environment = parser.parse();

The CLI writes the versioned JSON form for static sites:

.. code-block:: shell

   snt dip package --project DIPfile --output project.bundle.json

The bundle is limited to UTF-8 text within the DIPfile directory. Missing
files, path traversal, and symlinks escaping that directory are errors. Use
``--include relative/path`` for an additional file that a later override may
select through ``$source``. See :doc:`the Web integration
<../../integrations/web>` for ``parseBundle`` and ``validateBundle``.
