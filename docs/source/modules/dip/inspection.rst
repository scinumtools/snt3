Inspecting environments
=======================

An evaluated ``Environment`` contains the values and structure produced by
DIPL, along with available metadata and provenance. ``Inspector`` provides a
read-only way to explore that environment: find paths, examine values and
tables, locate their source, and trace recorded dependencies. Create one
inspector for an environment and reuse it for related queries. The environment
must outlive the inspector.

Use ``open_artifact`` to load a DIPfile, DIPL source, or DIPH5 snapshot. Use
``DIP`` when you need to assemble inputs before parsing. Comparison and preview
work across separate evaluations, so they are independent of ``Inspector``.
The tables below show which operation to use; the following sections give
short C++ examples.

Inspection quick reference
--------------------------

Start with an evaluated environment. Enable the two optional inspection records
only if you need them:

.. code-block:: cpp

   using namespace snt::dip;
   auto env = open_artifact("DIPfile", true, true);
   Inspector view{env};

The first ``true`` records dependencies; the second retains original array and
table blocks. The examples use illustrative paths. ``other`` means a second
environment; ``source``, ``overrides``, and ``error`` stand for caller inputs.

Find a path
~~~~~~~~~~~

.. list-table:: Finding information in an environment
   :header-rows: 1
   :widths: 40 60

   * - C++ call
     - What it provides and when to use it
   * - :cpp:func:`detect_artifact("DIPfile") <snt::dip::detect_artifact>`
     - Identifies an input by its filename. Use it before opening an unfamiliar
       project, DIPL file, or DIPH5 snapshot.
   * - :cpp:func:`open_artifact("DIPfile") <snt::dip::open_artifact>` / :cpp:func:`reload_artifact(env, "DIPfile") <snt::dip::reload_artifact>`
     - Loads an environment, or refreshes one after a file changes. A failed
       reload leaves the current environment intact.
   * - :cpp:func:`view.select("?physics.") <snt::dip::snt::dip::Inspector::select>`
     - Finds matching evaluated values. Use a path query or tag filter to
       narrow a search.
   * - :cpp:func:`view.cursor("physics").children() <snt::dip::snt::dip::Cursor::children>`
     - Walks direct children. Use :cpp:func:`elements() <snt::dip::snt::dip::Cursor::elements>` for lists and :cpp:func:`items() <snt::dip::snt::dip::Cursor::items>`
       for maps when building a tree.
   * - :cpp:func:`view.capabilities("physics.speed") <snt::dip::snt::dip::Inspector::capabilities>`
     - Reports which views a path supports: value, children, source, array,
       table, or graph. Use it to choose relevant actions in a UI.

Read values and data
~~~~~~~~~~~~~~~~~~~~

.. list-table:: Reading evaluated data
   :header-rows: 1
   :widths: 40 60

   * - C++ call
     - What it provides and when to use it
   * - :cpp:func:`view.value("physics.speed") <snt::dip::snt::dip::Inspector::value>`
     - Returns an owned value with its type, shape, units, metadata, and
       provenance. Use for a detailed value view. :cpp:func:`view.values() <snt::dip::snt::dip::Inspector::values>`
       returns all values in environment order.
   * - :cpp:func:`view.value_summary("samples") <snt::dip::snt::dip::Inspector::value_summary>`
     - Returns type, shape, size, units, and metadata without copying the
       array. Use it to decide how much data to show.
   * - :cpp:func:`view.value_slice("samples", {{0, 9}}) <snt::dip::snt::dip::Inspector::value_slice>`
     - Reads elements 0 through 9 of an evaluated array. Use for paging or
       plots; the range is inclusive and does not trigger lazy disk reads.
   * - :cpp:func:`view.table("measurements") <snt::dip::snt::dip::Inspector::table>`
     - Returns row count and ordered column descriptions without copying
       cells. Read a column with :cpp:func:`view.value_slice <snt::dip::snt::dip::Inspector::value_slice>`; :cpp:func:`view.tables() <snt::dip::snt::dip::Inspector::tables>`
       lists all tables.
   * - :cpp:func:`view.block_input("samples") <snt::dip::snt::dip::Inspector::block_input>`
     - Returns the original array or table text when a live parse retained
       it. Use to show input beside evaluated data; :cpp:func:`view.block_inputs <snt::dip::snt::dip::Inspector::block_inputs>`
       lists all retained blocks.

Explain a value
~~~~~~~~~~~~~~~

.. list-table:: Meaning and provenance
   :header-rows: 1
   :widths: 40 60

   * - C++ call
     - What it provides and when to use it
   * - :cpp:func:`view.describe("physics.speed") <snt::dip::snt::dip::Inspector::describe>`
     - Produces a bounded description of a value, group, collection, or
       table. Use in search results or agent tools. :cpp:func:`view.list_descriptions <snt::dip::snt::dip::Inspector::list_descriptions>`
       describes a limited set of query matches.
   * - :cpp:func:`view.override_contract("physics.speed") <snt::dip::snt::dip::Inspector::override_contract>`
     - Classifies a proposed override path and reports its type, current shape,
       units, and enforced rules. Use it to prepare an override before preview.
   * - :cpp:func:`view.source_locations(source) <snt::dip::snt::dip::Inspector::source_locations>`
     - Finds declarations, modifications, and overrides for a
       ``SourceEntity``. Use for source navigation; entities can also be
       named sources, schemas, units, or project entries.
   * - :cpp:func:`view.dependency_neighborhood("physics.speed") <snt::dip::snt::dip::Inspector::dependency_neighborhood>`
     - Shows direct inputs, readers, and current value and condition events.
       Use for a selected node's graph view; graph recording must be enabled.
   * - :cpp:func:`view.graph().dependencies("?physics.speed") <snt::dip::snt::dip::DependencyGraph::dependencies>`
     - Accesses recorded reads. The graph also exposes reverse references,
       evaluation history, and expression trees for calculation tracing.

Check a proposed change
~~~~~~~~~~~~~~~~~~~~~~~

.. list-table:: Comparing and reporting changes
   :header-rows: 1
   :widths: 40 60

   * - C++ call
     - What it provides and when to use it
   * - :cpp:class:`Comparison comparison{env, other} <snt::dip::snt::dip::Comparison>`
     - Finds added, removed, and changed values between two environments.
       Pass two DIPH5 paths to load snapshots instead. ``comparison.render()``
       gives a bounded text summary.
   * - :cpp:func:`preview("DIPfile", overrides) <snt::dip::preview>`
     - Evaluates a baseline and a candidate with proposed overrides. Use
       its diagnostics and comparison before applying a change.
   * - :cpp:func:`diagnostic_from_exception(error) <snt::dip::diagnostic_from_exception>`
     - Converts a caught exception into a structured message with available
       source locations. Use it to report parse or reload failures.

Using the inspection APIs
-------------------------

The examples in this section assume the ``env`` and ``view`` created above
and ``#include <snt/dip/inspect/inspector.h>``. Paths are illustrative;
use paths defined by your own DIPL input.

Open and reload an artifact
~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`snt::dip::detect_artifact` recognizes conventional filenames:
``DIPfile``, ``.dip`` or ``.dipl``, ``.dipt``, and ``.diph5``. It does not
validate contents. :cpp:func:`snt::dip::open_artifact` loads a project, DIPL
file, or DIPH5 snapshot. A ``.dipt`` file is table input within DIPL and
cannot be opened as an independent environment.

.. code-block:: cpp

   #include <snt/dip/artifact.h>

   auto kind = snt::dip::detect_artifact("DIPfile");
   auto env = snt::dip::open_artifact("DIPfile");
   snt::dip::reload_artifact(env, "DIPfile");

:cpp:func:`snt::dip::reload_artifact` replaces ``env`` only after the new
input loads successfully. Pass recording options again when reloading a live
parse: ``reload_artifact(env, "DIPfile", true, true)`` retains its dependency
graph and original input blocks. A loaded DIPH5 snapshot uses the graph saved
in the file.

Find values and traverse groups
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::select <snt::dip::snt::dip::Inspector::select>` finds
evaluated value nodes by query, optionally filtered by tags. A
:cpp:class:`Cursor <snt::dip::snt::dip::Cursor>` traverses
a known path. ``children()`` is for groups, ``elements()`` for lists, and
``items()`` for maps.

.. code-block:: cpp

   for (const auto& node : view.select("?physics.")) {
       // node->path.name is the full path of a matching value.
   }

   auto group = env["physics"];
   for (const auto& [name, child] : group.children()) {
       // child.get_path() identifies a direct child.
   }

A ``?`` query selects all values; ``?physics.`` selects a subtree. Selection
returns independent node snapshots, while cursors read the environment.
See :doc:`Basic DIP usage <basic-usage>` for tag-filter and collection examples.

Inspect owned values
~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::value <snt::dip::snt::dip::Inspector::value>` returns an owned typed value together
with shape, units, metadata, tags, source provenance, declaration and
override locations, and schema information. Use
:cpp:func:`Inspector::values <snt::dip::snt::dip::Inspector::values>` to obtain every evaluated value in
environment order, with its full path.

.. code-block:: cpp

   auto speed = view.value("physics.speed");
   auto shape = speed.shape;
   auto units = speed.units;
   auto source = speed.declaration_location;
   if (speed.override_location) {
       auto replacement = *speed.override_location;
   }

``speed.changes`` lists the declaration, applied modifications, and effective
override in evaluation order. Modifications ignored because of an override
are omitted. ``speed.contributing_schema`` identifies the schema that supplied
a value when known. A DIPH5 environment exposes only provenance retained in
its snapshot.

Locate source declarations
~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::source_locations <snt::dip::snt::dip::Inspector::source_locations>` accepts a ``SourceEntity`` for
an evaluated path, a path in a named source, a named source, schema, unit,
or DIPfile entry. For values it returns the effective override first, then
recent modifications, then the declaration.

.. code-block:: cpp

   auto locations = view.source_locations(
       {snt::dip::SourceEntityKind::Path, "physics.speed", {}});
   for (const auto& location : locations) {
       // location.source.path and location.line identify the physical source.
   }

Explicit groups and collection items have declaration locations. An inferred
parent without its own declaration has no invented location. Embedded text
points to its physical registration while ``logical_source_name`` and
``logical_line`` retain the original logical source. Distinct semantic
locations can share a physical line. Check ``source_text_available`` before
opening source text: DIPH5 may retain provenance without the complete text.

Inspect original array and table input
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A live parse can retain the original string blocks used for arrays and tables.
Enable this when opening or parsing, then inspect one path or list all blocks:

.. code-block:: cpp

   auto env = snt::dip::open_artifact("DIPfile", false, true);
   snt::dip::Inspector view{env};
   if (const auto* block = view.block_input("samples")) {
       auto original_text = block->code;
       auto source_line = block->source_line;
   }
   const auto& blocks = view.block_inputs();

Each block records its array or table kind, path, original text, logical
source, and line. Ordinary parses do not keep these copies. Use
``parser.parse(false, true)`` for a parser and pass the same option to
``reload_artifact`` after refresh. DIPH5 snapshots contain evaluated values,
not original block text.

Summarize and slice arrays
~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::value_summary <snt::dip::snt::dip::Inspector::value_summary>` reports type, shape, element
count, units, and metadata without cloning the value. Use
:cpp:func:`Inspector::value_slice <snt::dip::snt::dip::Inspector::value_slice>` for a bounded typed VAL value after
checking the shape:

.. code-block:: cpp

   auto summary = view.value_summary("samples");
   if (summary.elements >= 20) {
       auto page = view.value_slice("samples", {{10, 19}});
   }

Slice ranges are zero-based and inclusive. Both calls accept resolved paths
and named-source paths such as ``reference?calibration_curve``. They read
already evaluated in-memory values; DIPH5 loading remains eager and does not
provide disk-backed lazy reads.

Read table metadata and cells
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::table <snt::dip::snt::dip::Inspector::table>` reports row count and ordered column
metadata, including paths, types, and units, without copying cells. A table's
columns are evaluated arrays, so use :cpp:func:`Inspector::value_slice <snt::dip::snt::dip::Inspector::value_slice>`
on each column path. :cpp:func:`Inspector::tables <snt::dip::snt::dip::Inspector::tables>` lists all tables.

.. code-block:: cpp

   auto table = view.table("measurements");
   if (table.rows > 0) {
       for (const auto& column : table.columns) {
           auto first_cell = view.value_slice(column.path, {{0, 0}});
       }
   }

Table identity is retained in DIPH5 files produced by this version.

Check available views
~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::capabilities <snt::dip::snt::dip::Inspector::capabilities>` reports which facts and operations
are available for a value, group, collection, table, or named-source path.
Use it before showing a viewer action:

.. code-block:: cpp

   auto capabilities = view.capabilities("measurements");
   if (capabilities.hasTabularData) {
       auto table = view.table("measurements");
   }

``hasArrayData`` identifies array-valued nodes. ``hasReferenceGraph`` is true
when a value or its condition has recorded reads or an expression tree, or
another value references it. ``sourceEditable`` and ``directlyWritable``
remain false.

.. _cpp-semantic-interface:

Describe one path or search for descriptions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::describe <snt::dip::snt::dip::Inspector::describe>` gives a bounded description of a value, group,
collection, or table. It shares value and source facts with
``Inspector::value``, but omits large content. Scalars and small arrays appear
as DIPL text; ``value_unavailable_reason`` explains omitted content.
:cpp:func:`Inspector::list_descriptions <snt::dip::snt::dip::Inspector::list_descriptions>` applies the same description to a
``?`` query and tag filters, returning a limited list and total match count.

.. code-block:: cpp

   #include <snt/dip/inspect/semantic.h>

   auto one = view.describe("simulation.steps");
   auto matches = view.list_descriptions("?simulation.", {}, 50);
   // one.value_text is optional; matches.total includes all matches.

Descriptions separate enforced options and conditions from advisory metadata.
They report source-text and dependency-graph availability. List results omit
value content by default. A ``default`` is not separately evaluated; use a
preview for a baseline comparison.

Check an override target
~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::override_contract <snt::dip::snt::dip::Inspector::override_contract>`
checks an exact path against the evaluated model before you construct an
override. It identifies an existing value, an existing collection item, or a
schema backed item that can be created:

.. code-block:: cpp

   auto value = view.override_contract("simulation.steps");
   auto item = view.override_contract("materials[copper]");
   if (item.kind == snt::dip::OverrideTargetKind::NewItem) {
       // An item group can be written at item.path; preview checks its children.
   }

For a value, ``declared_type``, ``current_shape``, ``units``,
``enforced_options``, and ``enforced_condition`` describe known constraints.
For a collection item, ``item_schemas`` and ``resolved_path`` describe the
item at this evaluation: ``items[]`` resolves to the next list index. A
missing schema or mismatched selector yields ``Unavailable`` and a stable
``reason``. New child values cannot be targeted directly; create their item
group first. Contracts are static and may become stale if the input changes.
Use preview to validate proposed values, item children, and dependencies.

Preview an override
~~~~~~~~~~~~~~~~~~~

:cpp:func:`snt::dip::preview` parses a project or DIPL file independently for
the baseline and candidate, applying ordered override entries to the candidate.
Inspect validation diagnostics, accepted targets, and the comparison before
using an override:

.. code-block:: cpp

   #include <snt/dip/preview.h>

   std::vector<snt::dip::PreviewOverride> overrides{
       {snt::dip::PreviewOverride::Kind::Text, "simulation.steps = 200"}
   };
   auto result = snt::dip::preview("DIPfile", overrides);
   if (result.candidate_valid) {
       auto changed = result.comparison.changed;
   } else {
       auto diagnostics = result.candidate_diagnostics;
   }

Preview writes no project or generated output files. A DIPH5 snapshot can be
described, but cannot be a preview input. Results require stable source files
and pure host callbacks across the two evaluations.

Compare evaluated environments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:class:`snt::dip::Comparison <snt::dip::snt::dip::Comparison>` compares two loaded environments or two
DIPH5 snapshots. Its result can be inspected as structured differences or
rendered as bounded text.

.. code-block:: cpp

   #include <snt/dip/comparison.h>

   auto before = snt::dip::open_artifact("before.diph5");
   auto after = snt::dip::open_artifact("after.diph5");
   snt::dip::Comparison comparison{before, after};
   auto summary = comparison.render(20);

See :ref:`DIPH5 comparison <dip-diph5-comparison>` for comparison scopes and array
difference semantics.

Capture dependency events
~~~~~~~~~~~~~~~~~~~~~~~~~

Dependency recording is opt-in for live parses: use ``parser.parse(true)`` or
``open_artifact(path, true)``. Normal parsing skips graph allocation. Pass
``true`` again to ``reload_artifact`` after refreshing a live graph.

.. code-block:: cpp

   auto env = snt::dip::open_artifact("DIPfile", true);
   snt::dip::Inspector view{env};
   const auto& graph = view.graph();
   if (graph.recorded) {
       for (const auto& read : graph.dependencies("?physics.speed")) {
           // read.target is a resolved DIP node or source ID.
       }
       auto readers = graph.referenced_by("?physics.speed");
   }

``graph.events`` retains evaluation order and source locations. A value event
records reads made during assignment; condition and branch-decision events
are separate. ``dependencies`` uses the latest value event, excluding
superseded expressions; ``referenced_by`` performs the reverse lookup.
Each read retains its resolved target, original request, and EXS operand
text when present. Node IDs use ``?path`` for the current environment and
``source?path`` for imports; branch decisions use ``#case:N``. A value
event's ``controlled_by`` identifies active decisions that selected it.
Imported values retain a link to their source-qualified original.

Inspect a dependency neighborhood
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`Inspector::dependency_neighborhood <snt::dip::snt::dip::Inspector::dependency_neighborhood>` provides a one-hop
view for a selected path: distinct value and condition dependencies, values
whose latest evaluations read it, and separate latest value and condition
events. It also retains edge request and operand text.

.. code-block:: cpp

   auto neighborhood = view.dependency_neighborhood("physics.speed");
   if (neighborhood.recorded) {
       for (const auto& dependency : neighborhood.dependencies) {
           // dependency.id is a resolved node or source ID.
       }
       if (neighborhood.value_event &&
           neighborhood.value_event->composition) {
           const auto& operations = neighborhood.value_event->composition->nodes;
       }
   }

The input can be a resolved path, ``?path``, ``source?path``, or ``source?``.
When graph capture is unavailable, ``recorded`` is false and no links are
inferred. Numerical and logical expressions include an EXS composition tree
with operands, operators, and groups. Direct references and string templates
have reads without an EXS tree. Units stay on inspected values; their internal
expressions are not expanded into this graph.

Graph data persists in DIPH5 2.7 snapshots when it was recorded. Loading a
snapshot uses the saved graph state regardless of the recording option passed
to ``open_artifact`` or ``reload_artifact``. Older snapshots have no recorded
graph. Function callbacks contribute links only for DIP values requested
through the environment. A read means "read during evaluation," not
"necessary for the final result."

Convert exceptions to diagnostics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:func:`snt::dip::diagnostic_from_exception` converts a caught exception
to ``snt::core::Diagnostic`` without parsing formatted ``what()`` text. It
retains the category, message, details, suggestion, and available source
locations. Codes classify exception types, not individual error cases.

.. code-block:: cpp

   #include <snt/dip/inspect/diagnostic.h>

   try {
       snt::dip::reload_artifact(env, "DIPfile");
   } catch (const std::exception& error) {
       auto diagnostic = snt::dip::diagnostic_from_exception(error);
       // Show diagnostic.message and diagnostic.location to the user.
   }
