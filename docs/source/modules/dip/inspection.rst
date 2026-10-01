Inspecting and reloading environments
=====================================

The read-only ``<snt/dip/inspection.h>`` API collects facts already retained
by an evaluated environment. ``inspect_value`` returns an owned typed value,
shape, units, metadata, tags, declaration and override locations, and schema
information. ``inspect_values`` returns all evaluated values in environment
order with their full paths.

.. code-block:: cpp

   #include <snt/dip/inspection.h>

   auto env = snt::dip::open_artifact("DIPfile");
   auto speed = snt::dip::inspect_value(env, "physics.speed");
   auto shape = speed.shape;
   auto source = speed.declaration_location;
   if (speed.override_location) {
       auto replacement = *speed.override_location;
   }

The ``changes`` member lists the declaration, applied value modifications, and
an effective override in evaluation order, with source locations. Changes
ignored because of an override are not included. The ``provenance`` member
retains the existing source manifest details; ``contributing_schema``
identifies a schema that supplied the value when known. A loaded DIPH5
environment contains only provenance retained in the snapshot.

``detect_artifact(path)`` classifies conventional names: ``DIPfile``,
``.dip`` or ``.dipl``, ``.dipt``, and ``.diph5``. It does not validate file
contents. ``open_artifact(path)`` loads a project, DIPL file, or DIPH5 snapshot.
``.dipt`` is currently table input used within DIPL, so it cannot be opened
as an independent environment.

To refresh after an external edit, use ``reload_artifact``. It builds a fresh
environment and replaces the current one only when parsing or loading succeeds:

.. code-block:: cpp

   snt::dip::reload_artifact(env, "DIPfile");

For in-memory arrays, ``read_value_slice`` accepts zero-based inclusive
ranges and returns a typed VAL value containing the selected elements:

.. code-block:: cpp

   auto values = snt::dip::read_value_slice(env, "samples", {{10, 19}});

This slices an already evaluated value. DIPH5 loading remains eager; this API
does not provide disk-backed lazy reads.

Tables are represented by evaluated column arrays. ``inspect_table`` exposes
their shared row count and ordered column metadata, including types and units,
without copying column values. Use ``read_value_slice`` with a column path to
read a range. ``inspect_tables`` lists the tables in an environment. Table
identity is retained when saving and loading DIPH5 files produced by this
version.

.. code-block:: cpp

   auto table = snt::dip::inspect_table(env, "measurements");
   if (table.rows > 0) {
       for (const auto& column : table.columns) {
           auto first_row = snt::dip::read_value_slice(env, column.path, {{0, 0}});
       }
   }

``inspect_capabilities`` reports which facts and operations are available at
a value, group, collection, or table path. For example, ``hasTabularData`` is
true at a table path and ``hasArrayData`` is true for array-valued nodes.
``hasReferenceGraph`` is true when an evaluated value or its condition has
recorded references or an expression tree. ``sourceEditable`` and
``directlyWritable`` remain false.

.. code-block:: cpp

   auto capabilities = snt::dip::inspect_capabilities(env, "measurements");
   if (capabilities.hasTabularData) {
       auto table = snt::dip::inspect_table(env, "measurements");
   }

Dependency and operation graph
------------------------------

Dependency recording is opt-in. Use ``parser.parse(true)`` or
``open_artifact(path, true)`` when the graph is needed. Ordinary parsing skips
graph allocation and keeps the EXS solver on its graph-free path. Pass ``true``
again to ``reload_artifact(env, path, true)`` when refreshing a graph view.

``env.dependency_graph()`` retains DIP evaluation events in order. Its
``recorded`` flag distinguishes a captured graph from an ordinary parse. A value
event records the nodes read while assigning a value; condition and branch
decision events are separate. Node IDs use ``?path`` for the current
environment and ``source?path`` for imported sources. ``dependencies`` returns
reads from the latest value event, so a replaced expression does not appear as
a current dependency. ``referenced_by`` provides the reverse lookup. Earlier
events remain available in ``events`` with their source locations for provenance.
Each read retains its resolved target, request, and EXS operand text when present.
Branch decision events use ``#case:N`` IDs. A value event's ``controlled_by``
list identifies the active decisions that selected it. Imported value nodes
retain a read link to the original node, including its source qualifier.

Numerical and logical expressions include the EXS composition tree produced
by the same evaluation pass. Its operands, operators, and groups show how the
calculation is assembled. String templates and direct references have node
reads without an EXS operation tree. Units and quantities remain parameters
on inspected values; their internal unit expressions are not expanded into
this graph.

.. code-block:: cpp

   auto env = snt::dip::open_artifact("DIPfile", true);
   const auto& graph = env.dependency_graph();
   for (const auto& read : graph.dependencies("?physics.speed")) {
       // read.target is the resolved DIP node ID.
   }
   const auto* calculation = graph.latest("?physics.speed", snt::dip::DependencyEventKind::Value);
   if (calculation && calculation->composition) {
       const auto& operations = calculation->composition->nodes;
       const auto root = calculation->composition->root;
   }

The graph is retained in DIPH5 2.7 snapshots when recording was enabled.
Loading a snapshot uses its saved graph state, regardless of the recording
option passed to ``open_artifact`` or ``reload_artifact``. Older snapshots load
normally, but have no recorded graph. A function callback contributes node
links only for DIP values it actually requests through the environment.
Conditional expression operands are evaluated according to the current EXS
semantics; the links mean "read during evaluation," not "necessary for the final
result."

Errors can be converted to the shared ``snt::core::Diagnostic`` type without
parsing formatted exception text. It retains the exception category, message,
details, suggestion, and available source locations. Current codes classify
exception types, rather than individual error cases.

.. code-block:: cpp

   #include <snt/dip/diagnostic.h>

   try {
       snt::dip::reload_artifact(env, "DIPfile");
   } catch (const std::exception& error) {
       auto diagnostic = snt::dip::diagnostic_from_exception(error);
       // Show diagnostic.message and diagnostic.location to the user.
   }
