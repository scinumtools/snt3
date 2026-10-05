Python binding
==============

The ``scinumtools3`` package exposes SNT modules to Python. It uses the same
C++ implementation as the command-line and CMake interfaces, so PUQ and DIP
expressions behave the same way in Python and C++.

Import the classes you need from their respective submodules, as shown below.

Quantities and units
--------------------

PUQ combines numerical values with physical units and optional uncertainties.
You can add, multiply, compare, format, and convert quantities while retaining
their dimensions. This helps catch calculations that mix incompatible units.

.. code-block:: python

   from scinumtools3.puq import Quantity
   from scinumtools3.dip import DIP

   length = Quantity(2.5, "m")
   print(length.convert("cm"))

DIPL parameters
---------------

The Python binding can load a DIPL file, evaluate its parameters, and make
their values available as Python objects. For example, given
``parameters.dip``:

.. code-block:: dipl

   length float = 2.5 dm
   width float = 40 mm
   area float = ( {?length} * {?width} ) m2

Add the file to a parser, then evaluate it into an environment:

.. code-block:: python

   from pathlib import Path
   from scinumtools3.dip import DIP

   dip = DIP()
   dip.add_file(Path("parameters.dip"))
   env = dip.parse()

For a DIPfile project, ``add_project`` registers its files, sources, and units
from the manifest:

.. code-block:: python

   dip = DIP()
   dip.add_project("DIPfile")
   env = dip.parse()

See :doc:`DIPfile projects <../modules/dip/projects>` for the manifest format.

Compare saved snapshots with ``compare_diph5`` or loaded environments with
``compare``. The default scope compares effective values; ``Full`` also
compares persisted metadata and provenance.

.. code-block:: python

   from scinumtools3.dip import (
       ComparisonOptions, ComparisonScope, compare_diph5, render_comparison
   )

   options = ComparisonOptions()
   options.scope = ComparisonScope.Full
   result = compare_diph5("before.diph5", "after.diph5", options)
   print(render_comparison(result, max_details=20))
   print(result.equal, result.added, result.removed, result.changed)

The command layer also provides ``scinumtools3.api.dip.DIPCompare``. See
:doc:`DIPH5 comparison <../modules/dip/comparison>` for the compared fields.

Application adapters
--------------------

Subclass ``Adapter`` to turn an evaluated environment into files for an
application. As in C++, an adapter can register text, binary data, and a
streaming callback that writes bytes. ``run_adapter_project`` parses a
DIPfile, ``run_adapter_snapshot`` loads a DIPH5 snapshot, and ``run_adapter``
uses an existing environment.

.. code-block:: python

   from scinumtools3.dip import Adapter, run_adapter_project

   class AnalysisAdapter(Adapter):
       def plan(self, env, context):
           steps = env["run.steps"].value
           context.add_text("job.json", '{"steps": %d}\n' % steps)
           context.add_binary("marker.bin", b"SNT3")

           def write_steps(write):
               for i in range(steps):
                   write(f"{i}\n".encode())

           context.add_stream("steps.csv", write_steps)

   run_adapter_project("DIPfile", AnalysisAdapter(), "analysis-inputs")

See :doc:`Application adapters <../modules/dip/adapters>` for path rules and
the complete C++ and Python examples.

Accessing nodes
---------------

Use either of these approaches to access nodes:

* **Cursor:** use ``env["path"]`` when you know the path, or to traverse its
  groups and collections.
* **Select:** use ``env.select(...)`` to find value nodes by path and tags.
  Each result is an independent snapshot.

Cursor: inspect a known path
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   area = env["area"]
   print(area.value, area.units)  # 0.01 m2
   print(area.metadata.description)

A cursor points to a path in the environment. Use ``cursor.elements()`` to
traverse a list and ``cursor.items()`` to traverse named map items. Cursors
also expose ``shape``, ``metadata``, ``provenance``, and ``to_numpy()``.

For a unitless scalar such as ``count int = 42``, read
``env["count"].value``. For a value with units, a cursor exposes both ``value``
and ``units``. Pass them to ``Quantity`` when you need an explicit conversion:

.. code-block:: python

   from scinumtools3.puq import Quantity

   area_in_cm2 = Quantity(area.value, area.units.to_string()).convert("cm2")
   print(area_in_cm2)  # 100 cm2

Select: discover and inspect nodes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Select returns value nodes with their values, units, tags, and metadata:

.. code-block:: python

   dip = DIP()
   dip.add_string('''physics
     speed float = 2 m/s
       !tags ["export", "runtime", "hydro"]
       ?descr "Flow speed"
   ''')
   env = dip.parse()
   for node in env.select(
       "?physics.",
       tags_all=["export", "runtime"],
       tags_any=["hydro", "gravity"],
       tags_none=["internal", "deprecated"],
   ):
       print(node.name, node.value, node.units, node.tags, node.metadata.description)

``tags_all`` requires every listed tag; ``tags_any`` requires at least one;
and ``tags_none`` excludes nodes with any listed tag. The filters combine
with AND. An omitted or empty filter imposes no restriction. Only tags
assigned directly to a node match; parent tags are not inherited.

``?`` selects every value node. ``?physics.`` selects a subtree, including a
value-bearing root and collection members. ``?physics.speed`` selects one
exact node. Results retain fully qualified paths and environment order, and
each matching node appears once. If nothing matches, the result is an empty
list.

Selection does not modify the environment. A selected node is an independent
snapshot, and its ``tags`` and ``metadata`` properties are read-only. Changing
a returned tags list affects only that list. Metadata remains valid while its
Python wrapper is alive.

Additional request helpers remain available in the
:doc:`Python DIP API reference <../api/python_dip>`.

Registering schemas
-------------------

Register a schema body directly, then apply it in ordinary DIPL code:

.. code-block:: python

   dip = DIP()
   dip.add_schema_string("settings", "speed float = 2 m/s\n")
   # Alternatively: dip.add_schema_file("settings", Path("settings.dipl"))
   dip.add_string("physics : settings")
   env = dip.parse()

Schema bodies start at indentation zero and omit the ``$schema`` wrapper.
Registration retains source information; values are evaluated when the schema
is applied. Put schema-level ``?`` metadata before the first body node and
inspect it through ``env.schemas["settings"].metadata``. This metadata
describes the schema definition. It is not copied onto the group, collection,
item, or value node where the schema is applied.

DIPH5 stores evaluated nodes and descriptive schema provenance, but not
reusable schema definitions. After loading a snapshot, use
``env.schema_manifest`` for schema descriptions and citations.
``env.applied_schemas("physics.speed")`` lists schemas applied along a value's
path, while ``env.contributing_schema("physics.speed")`` identifies the schema
that supplied the value node, if any. Selected value nodes also expose its
``schema_id``. Each schema record contains ``metadata``, ``source_name``,
``source_line``, and an optional ``source`` identity with a path and hash.
``env.schemas`` remains empty after loading, so a snapshot cannot instantiate
schemas.

The command API also accepts ``argument_add("schema_string", [name, body])``
and ``argument_add("schema_file", [name, path])``. Pass file paths as strings.

Overriding values
-----------------

Use a ``$override`` region in DIPL, or register an unwrapped override body
before ``parse()``:

.. code-block:: python

   dip = DIP()
   dip.add_override_string("simulation.steps = 1024")
   # Alternatively, read an unwrapped body from a file:
   # dip.add_override_file("overrides.dip")
   dip.add_string("simulation\n  steps int = 100")
   env = dip.parse()
   print(env["simulation.steps"].value)  # 1024

An override body contains ``path = value`` modifications and may use nested
path prefixes such as ``"simulation\n  steps = 1024"``. Both string and file
bodies start at indentation zero. Each expanded path may appear only once and
must have a normal declaration. Override values can also use DIPL references,
expressions, and registered value functions. For example, call
``dip.add_override_string("radius = ({?base} * 2) cm")``. The replacement is
evaluated at the target declaration, so its dependencies must already be
available. See :ref:`dip-overrides` for more examples, evaluation order, and
unit conversion rules.

Overrides can replace ``!constant`` values during initial evaluation. Later
ordinary modifications are ignored, while dependent expressions and
conditions see the replacement. An existing condition may then activate or
deactivate nodes. An override whose target is never instantiated produces an
unresolved override error. You can also register overrides after
``add_project("DIPfile")``; if an override body is rejected, it registers no
entries.

Inspect ``node.override`` on a selected node and
``cursor.provenance.override_source`` to find the replacement's origin. DIPH5
preserves both the effective value and its override provenance.

Environment persistence
-----------------------

Save evaluated parameters to DIPH5 and restore them with ``Environment``:

.. code-block:: python

   from scinumtools3.dip import DIP, Environment

   parser = DIP()
   parser.add_string("simulation.steps int = 100")
   env = parser.parse(record_dependency_graph=True)
   env.save("parameters.diph5")

   restored = Environment()
   restored.load("parameters.diph5")
   assert restored["simulation.steps"].value == 100

See :doc:`Environment persistence <../modules/dip/persistence>` for the file
format, save and load behavior, and current limitations.

The command API supports the same workflow. Configure ``argument_save()`` or
``argument_load()``; the file is written or read when you call ``execute()``:

.. code-block:: python

   from scinumtools3.api.dip import DIPParse

   save = DIPParse()
   save.argument_add("string", ["simulation.steps int = 100"])
   save.argument_save("parameters.diph5")
   save.execute()

   load = DIPParse()
   load.argument_load("parameters.diph5")
   load.argument_request("simulation.steps")
   load.argument_value("integer")
   assert load.execute() == "100\n"

``argument_load()`` cannot be combined with ``argument_add()``. Saving
overwrites the destination with the full environment, regardless of request
or tag filters, and requires successful output validation. These command
methods take file paths as strings; use ``str(path)`` for a ``pathlib.Path``.

Generating static parameters
----------------------------

Use ``Environment.generate()`` to export validated DIPL parameters as source
or data files for another application. Import ``ExportFormat`` from
``scinumtools3.dip`` and choose ``CPP``, ``C``, ``FORTRAN``, ``RUST``,
``JULIA``, ``JSON``, or ``YAML``:

.. code-block:: python

   from scinumtools3.dip import DIP, ExportFormat

   parser = DIP()
   parser.add_string("simulation.steps int = 100")
   env = parser.parse()
   env.generate(ExportFormat.CPP, "parameters.hpp")
   env.generate(ExportFormat.JSON, "parameters.json")

The command API can also generate a file during ``execute()``:

.. code-block:: python

   from scinumtools3.api.dip import DIPParse

   generate = DIPParse()
   generate.argument_add("file", ["parameters.dip"])
   generate.argument_generate("rust", "parameters.rs")
   generate.execute()

Generation always exports the complete evaluated environment. Requests and
tags restrict only textual output. See :doc:`Static parameter generation
<../modules/dip/generation>` for native representations and format-specific
behavior.

Generating reports
------------------

After parsing a project or loading a DIPH5 snapshot, call
``Environment.generate_report()`` to create a report. ``ReportFormat``
supports TeX, PDF, Markdown, reStructuredText, HTML, Typst, plain text, and
Brief++ document JSON:

.. code-block:: python

   from scinumtools3.dip import DIP, ReportFormat

   parser = DIP()
   parser.add_project("DIPfile")
   env = parser.parse()
   env.generate_report(ReportFormat.TEX, "report.tex",
                     input_label="DIPfile", intro_file="introduction.tex")
   env.generate_report(ReportFormat.PDF, "report.pdf",
       input_label="DIPfile", intro_file="introduction.tex",
       title="Mock Heat Flow Study", author="Example Research Team",
       date="2026-09-28", version="1.0 demo")
   env.generate_report(ReportFormat.HTML, "report.html", input_label="DIPfile")
   env.generate_report(ReportFormat.MARKDOWN, "report.md")

The remaining format values are ``RST``, ``TYPST``, ``TEXT``, and ``JSON``.
Markdown uses MyST-style table directives, and JSON contains a ``briefpp/1``
document tree. These text formats need no external tools.
The report keeps a full entry for every parameter. Its Parameter guide links
to entries in HTML and PDF; the Schemas section lists parameters supplied by
each schema. Parsing with ``record_dependency_graph=True`` adds evaluated
expressions, direct reads, branch selections, and readers to the relevant
entries. A loaded DIPH5 environment uses its saved graph if present. Without
a graph, the report states that calculation relationships are unavailable.

Use ``intro_file`` to supply a trusted LaTeX fragment without a preamble for
TeX or PDF output. PDF generation requires a local TeX compiler; set
``tex_compiler="lualatex"`` or another compatible executable if needed. TeX
output itself needs no external tool. Generated PDFs include a cover and a
linked contents page.

You can also generate a report from an ``Environment`` restored with
``load()``. Such a report includes only provenance retained in DIPH5. The
default title is ``DIP parameter report``; an empty author appears as
``Not specified``. The date and version default to the local generation date
and SNT build version. See the :ref:`CreateReport example
<dip-create-report-example>` for a PDF and :doc:`Environment persistence
<../modules/dip/persistence>` for DIPH5 limits.

Source provenance
-----------------

Each value cursor has a read-only ``provenance`` object with the source name,
line number, captured source text, and DIPL citation metadata. DIPH5 version
2 also preserves a source manifest. Access it through
``environment.source_manifest`` to inspect source paths and SHA-256 content
fingerprints:

.. code-block:: python

   provenance = env["simulation.steps"].provenance
   print(provenance.source_name, provenance.source_line)
   print(provenance.metadata.doi)
   if provenance.source is not None:
       print(provenance.source.path, provenance.source.hash)

If a value was overridden, ``provenance.override_source``,
``provenance.override_line``, and ``provenance.override_code`` identify its
effective origin. The original declaration fields remain available.

Inspecting environments
-----------------------

Values and tables
~~~~~~~~~~~~~~~~~

The read-only DIP inspection API exposes stable paths, effective values,
units, metadata, source locations, applied changes, and schema information.
``inspect_values(env)`` returns values in environment order; each value's
``changes`` list follows evaluation order. Use
``inspect_capabilities(env, path)`` to check whether a path contains a value,
children, an array, or a table. The direct editing flags remain false.

.. code-block:: python

   from scinumtools3.dip import inspect_capabilities, inspect_table, inspect_value, read_value_slice

   if inspect_capabilities(env, "measurements").has_tabular_data:
       table = inspect_table(env, "measurements")
       for column in table.columns:  # original DIPL header order
           if table.rows:
               values = read_value_slice(env, column.path, [(0, table.rows - 1)])
               print(column.name, column.units, values)

   speed = inspect_value(env, "physics.speed")
   print(speed.value, speed.declaration_location, speed.override_location)

``inspect_tables(env)`` lists all tables. A table inspection object gives its
column metadata and row count; column values remain available at their own
paths. Slice ranges are zero-based and inclusive, and read from an evaluated
in-memory value. DIPH5 loading is still eager.

Dependency graphs
~~~~~~~~~~~~~~~~~

Dependency recording is off by default. Enable it with
``parser.parse(record_dependency_graph=True)`` or
``open_artifact(path, record_dependency_graph=True)``. Pass the option again
to ``reload_artifact`` when refreshing parsed source.
``inspect_dependency_graph(env)`` returns node reads and numerical or logical
operation trees. Its ``recorded`` flag distinguishes a captured graph from an
ordinary parse.

.. code-block:: python

   from scinumtools3.dip import DIP, DependencyEventKind, inspect_dependency_graph

   parser = DIP()
   parser.add_string(
       "distance float = 12 m\n"
       "time float = 3 s\n"
       "speed float = ({?distance} / {?time}) m/s\n"
   )
   env = parser.parse(record_dependency_graph=True)
   graph = inspect_dependency_graph(env)
   print([edge.target for edge in graph.dependencies("?speed")])
   event = graph.latest("?speed", DependencyEventKind.Value)
   if event is not None and event.composition is not None:
       print(event.composition.root, len(event.composition.nodes))

Node IDs use ``?path`` for the current environment and ``source?path`` for
imports. ``dependencies`` reports reads from the latest value evaluation;
``events`` also retains earlier evaluations and branch decisions. A saved
DIPH5 snapshot retains its recorded graph, and loading uses that saved state
regardless of the ``record_dependency_graph`` option. See :doc:`the DIP
inspection guide <../modules/dip/inspection>` for graph semantics and
limitations.

Reload and diagnostics
~~~~~~~~~~~~~~~~~~~~~~

``open_artifact(path)`` loads a DIPfile, DIPL file, or DIPH5 snapshot.
``reload_artifact(env, path)`` replaces an environment only after the new
artifact loads successfully. DIP exceptions have a ``diagnostic`` attribute
with a structured category, message, details, suggestion, and available
source locations:

.. code-block:: python

   from scinumtools3.dip import diagnostic_from_exception, reload_artifact

   try:
       reload_artifact(env, "DIPfile")
   except RuntimeError as error:
       diagnostic = diagnostic_from_exception(error)
       if diagnostic is not None:
           print(diagnostic.code, diagnostic.message, diagnostic.location)

Python values and NumPy
-----------------------

The C++ VAL layer maps to familiar Python types: ``int``, ``float``, ``str``,
``bool``, lists, and ``numpy.ndarray``. Results therefore work directly with
Python and NumPy; there is no separate VAL value class to learn.

Why EXS is not exposed
----------------------

PUQ and DIP use EXS internally, but there is no standalone EXS Python module.
Python has tools for custom expression evaluation and grammars. A direct EXS
binding would mainly help when the same custom language must run in Python,
C++, the CLI, and REST services.

For application operations, use the package's Python API helpers. They
evaluate PUQ and DIPL definitions through the same implementation as the
``snt`` command-line tool and the C++ API. Use the binding when a Python
program needs typed results or direct access to module objects without
starting a subprocess.

For installation options and the complete Python API, see the
`Python binding README <https://github.com/scinumtools/snt3/tree/main/bindings/python>`_.
