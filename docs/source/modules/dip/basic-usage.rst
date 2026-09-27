Basic Usage
===========

The usual C++ DIP workflow is to create a ``DIP`` parser, add one or more DIPL
inputs, and call ``parse()``. Parsing evaluates the definitions and returns an
``Environment`` containing the resulting value nodes.

The file-based calls below assume two separate files in the program's current
working directory. Relative filenames are resolved from that directory.

``parameters.dipl``:

.. code-block:: dipl

   width float = 40 cm

``shared.dipl``:

.. code-block:: dipl

   scale float = 2

Parsing inputs
--------------

.. code-block:: cpp

   snt::dip::DIP dip;
   dip.add_string("length float = 2.5 m");
   dip.add_file("parameters.dipl");
   dip.add_unit("length_unit", "23*au");
   dip.add_source("shared", "shared.dipl");

   snt::dip::Environment env = dip.parse();
   double length = env["length"].as<double>();
   env.save("parameters.diph5");

``add_string`` and ``add_file`` queue definitions for evaluation by
``parse()``. In contrast, ``add_source(name, file)`` reads and evaluates an
external DIPL file immediately in a separate source environment. Its values
can then be referenced using expressions such as ``{shared?scale}``; they
are not automatically added to the local node tree. Register custom units
with ``add_unit(name, expression)`` before parsing the local definitions.

For a reusable :doc:`DIPfile project <projects>`, load the complete manifest
instead of registering its units, sources, and code inputs individually:

.. code-block:: cpp

   snt::dip::DIP dip;
   dip.add_project("DIPfile");
   snt::dip::Environment env = dip.parse();

Environment
-----------

Use two main approaches to access nodes in an environment, in both C++ and Python:

* **Cursor:** inspect a known path and traverse groups and collections.
* **Select:** discover value nodes by path and tags, returning independent
  snapshots with their original fully qualified paths.

Use ``env.load("parameters.diph5")`` to restore a previously persisted evaluated
environment without parsing the original DIPL files.

Cursor: inspect a known path
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Access a path using ``env["path.to.value"]``. The
resulting cursor exposes the evaluated value and its units; groups, maps, and
lists can be traversed using ``items()`` and ``elements()``.

``env["path.to.value"].get_provenance()`` returns the source name, line,
captured source line, and citation metadata for a resolved value. When the
environment was parsed directly or loaded from a DIPH5 version 2 or later file, the
result also includes the matching source-manifest entry with its recorded path
and SHA-256 content fingerprint.

Numeric values are retained internally at their DIPL storage precision. The
C++ cursor can return a lower native type when needed by an application:

.. code-block:: cpp

   int8_t layer_count = env["simulation.layers"].as<int8_t>();
   std::vector<float> samples = env["simulation.samples"].as<std::vector<float>>();

Lower signed, unsigned, and floating-point targets are converted with the
ordinary C++ cast rules. This does not change the values retained by the
environment.

Select: discover and inspect nodes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use ``select()`` to find value nodes and inspect their tags and metadata directly:

.. code-block:: cpp

   snt::dip::DIP dip;
   dip.add_string(
       "physics\n"
       "  speed float = 2 m/s\n"
       "    !tags [\"export\", \"runtime\", \"hydro\"]\n"
       "    ?descr \"Flow speed\"\n"
   );
   auto env = dip.parse();
   snt::dip::TagFilter filter;
   filter.all = {"export", "runtime"};
   filter.any = {"hydro", "gravity"};
   filter.none = {"internal", "deprecated"};
   for (const auto& node : env.select("?physics.", filter)) {
       std::cout << node->path.name << ": " << node->to_string()
                 << " — " << node->metadata.description << '\n';
       for (const auto& tag : node->tags)
           std::cout << "  " << tag << '\n';
   }

``all`` requires every listed tag, ``any`` requires at least one, and ``none``
excludes nodes with any listed tag. The three filters combine with AND; empty
fields impose no restriction. Tags match explicit assignments without
parent-tag inheritance.

Keep tags as classification labels. Put structured settings in typed nodes and
schemas, and descriptive information in metadata; see :ref:`dipl-tag-design`.

``?`` selects all value nodes, ``?physics.`` selects a subtree including its
value-bearing root and collection members, and ``?physics.speed`` selects an
exact node. Source-qualified queries such as ``shared?scale`` are also supported.
Results retain the original paths within the selected environment, in environment
order, with each matching node returned once. No matches returns an empty list.

Selection does not modify the environment. Each returned node is an independent
snapshot: changes to it do not affect the environment, and later environment
changes do not update the snapshot. In Python, the same filters are keyword
arguments named ``tags_all``, ``tags_any``, and ``tags_none``; see the
:doc:`Python guide <../../integrations/python>` for examples.

Additional APIs
~~~~~~~~~~~~~~~

For custom behavior, DIP also supports registered value- and node-producing
functions. Those functions are evaluated while parsing and are not part of a
persisted DIPH5 environment; applications that load DIPH5 should treat the
file as evaluated data. The complete class and function reference is available
in the :doc:`C++ DIP API <../../api/cpp_dip>` documentation.

For Python, C, command-line, and CMake usage, see
:doc:`Interfaces and integrations <../../integrations/index>`.

Registering schemas from host code
----------------------------------

Use ``add_schema_string(name, body)`` or ``add_schema_file(name, path)`` to
register one reusable schema. Supply its body without a ``$schema`` wrapper,
starting at indentation zero. Registration parses the schema immediately;
values and references are evaluated when the schema is applied during ``parse()``.
Place ``?`` metadata properties before the first body node to describe the
schema definition. Its metadata are available through ``env.schemas.at(name)``
and are not copied onto the applying group, collection, item, or values.
Original source text, file paths, line numbers, and source identities are retained.

.. code-block:: cpp

   snt::dip::DIP dip;
   dip.add_schema_string("settings", "speed float = 2 m/s\n");
   // Alternatively: dip.add_schema_file("settings", "settings.dipl");
   dip.add_string("physics : settings");
   auto env = dip.parse();

Schema names follow DIPL keyword rules and must be unique. Empty bodies and
bodies containing a surrounding ``$schema`` declaration are rejected. Ordinary
``add_string()`` and ``add_file()`` still accept complete DIPL documents.

.. _dip-overrides:

Overriding initial values
-------------------------

Use ``$override`` to replace values without changing a node's type, units,
properties, or hierarchy. The region contains only value modifications, with
fully qualified paths. Declare regions at the top level, outside schemas and
nested blocks. Their order among top-level inputs does not matter:

.. code-block:: dipl

   $override
     simulation.resolution = 1024

   simulation
     resolution int = 512
     cells int = ({?simulation.resolution} * 2)

Replacement values may be literals, references, expressions, or calls to
registered DIPL value functions. For example:

.. code-block:: dipl

   base float = 10 cm
   radius float = 1 cm
   diameter float = ({?radius} * 2) cm

   $override
     radius = ({?base} * 2) cm

This evaluates ``radius`` to ``20 cm`` and ``diameter`` to ``40 cm``.
Alternatively, the replacement could be ``radius = {?base} cm`` or
``radius = compute_radius() cm``, provided the host has registered the
``compute_radius`` value function. These are alternatives for the same target,
not entries to combine in one input.

An override is evaluated when its target declaration is processed. Values
referenced by its expression, reference, or function must already be available
at that point. Moving the ``$override`` region does not change this requirement
or enable forward references. The original value expression or function is
not evaluated; subsequent dependents see the replacement value.

The replacement must satisfy the target's type, declared dimensions, and
constraints. Explicit compatible units are converted to the target's declared
units; without explicit units, the replacement uses the target's units.
These rules also apply to bodies supplied through ``add_override_string`` and
``add_override_file``.

When an overridden value violates an options, condition, or format constraint,
the diagnostic points to the override and includes the original declaration
as context. File paths are included when available.

Dependencies and conditions see the overridden value. Existing conditional
definitions may therefore instantiate different nodes: overrides preserve the
declared model, while its evaluated graph follows its existing conditions. An
override targeting a node that remains inactive is an error.

An override may replace a ``!constant``
value; subsequent ordinary modifications to that node are ignored. Duplicate
or unmatched override paths are errors. Host code can call
``add_override_string(body)`` or ``add_override_file(path)``. Both accept an
unwrapped DIPL override body; files retain their source path in provenance. Selected nodes
expose the ``override`` flag, and cursor provenance retains both the original
declaration and the override source. DIPH5 persists the evaluated value and
this provenance.
