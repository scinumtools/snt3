DIP examples
============

The DIP examples cover a minimal DIPL definition, CMake configuration,
inspection and dependency graphs, application adapters, and report generation.

Quick Example
-------------

The ``examples/dip/QuickExample`` directory contains a small DIPL definition
with typed parameters, units, references, and validation properties. It is a
smoke test: the C++ and Python quick-example tests load the README example and
verify that it parses and evaluates successfully.

See the `QuickExample source directory <https://github.com/scinumtools/snt3/tree/main/examples/dip/QuickExample>`_.
The test does not exercise a separate application workflow; it checks that
this compact definition remains valid and produces an evaluable parameter
tree.

.. code-block:: dipl

   length float = 2.5 m
   width float = 40 cm
   area float = ({?length} * {?width})
     !condition ({.} > 0 m2)

CMake Integration
-----------------

The ``examples/dip/CMakeIntegration`` directory is a standalone CMake
consumer. It reads boolean, integer, and string values from ``build.dip``
with ``snt_dip_get`` and uses them to configure a C++ target and its standard.

See the :doc:`../integrations/cmake` guide for the commands and configuration
details. The complete source is available in the
`CMakeIntegration directory <https://github.com/scinumtools/snt3/tree/main/examples/dip/CMakeIntegration>`_.

The example shows configure-time evaluation with ``snt_dip_get``. A boolean
controls whether the executable target is created, an integer selects its C++
standard, and a string is printed during configuration. Editing ``build.dip``
causes CMake to reconfigure and apply the new settings.

.. code-block:: cmake

   snt_dip_get(FILE build.dip PATH build.tools TYPE BOOL OUTPUT BUILD_TOOLS)
   snt_dip_get(FILE build.dip PATH build.standard TYPE INTEGER OUTPUT CXX_STANDARD)
   if(BUILD_TOOLS)
       add_executable(dip-cmake-example main.cpp)
       set_property(TARGET dip-cmake-example PROPERTY CXX_STANDARD "${CXX_STANDARD}")
   endif()

Inspection and Dependency Graph
-------------------------------

The `InspectionGraph source directory
<https://github.com/scinumtools/snt3/tree/main/examples/dip/InspectionGraph>`_
contains a DIPfile with an override, a calculated speed, a condition, a branch,
a table, and an array. Its C++ program enables optional graph recording and
prints provenance, a bounded array slice, forward and reverse dependencies,
and the nested expression tree.

From the repository root, after building the example target:

.. code-block:: console

   cmake --build build --target ExampleDipInspectionGraph
   build/bin/ExampleDipInspectionGraph

The output connects the calculated value to its source nodes and shows the
expression structure:

.. code-block:: text

   Speed reads:
     ?experiment.distance -> ?experiment.distance
     ?experiment.duration -> ?experiment.duration
   Speed expression:
   operator div
     operand {?experiment.distance}
     operand {?experiment.duration}

See :doc:`the inspection guide <../modules/dip/inspection>` for the meaning of
the graph's ``?path`` IDs and its recorded events.

.. _dip-parameter-viewer-example:

Parameter Viewer
----------------

The `ParameterViewer source directory
<https://github.com/scinumtools/snt3/tree/main/examples/dip/ParameterViewer>`_
contains a self-contained DIPfile project designed for browsing. It combines
groups, a value with a child, map and list collections, a schema, two DIPL
sources and two raw sources, a custom unit, an override, typed arrays and two
tables, validation, metadata, expressions, a formatted string, a modification,
and a conditional branch. The project uses synthetic values and needs no data
files outside the example directory.

Build the optional viewer, then open the project from the repository root.
See :doc:`the viewer guide <../integrations/viewer>` for build dependencies and
browser behavior:

.. code-block:: console

   cmake -S . -B build -DENABLE_SNT_VIEW=ON
   cmake --build build --target parameter-viewer-snapshot
   build/bin/snt view examples/dip/ParameterViewer/DIPfile

The example also includes ``parameters.diph5``, an evaluated snapshot of
the same project. Open it with ``snt view`` to compare live-project and snapshot
browsing. A normal CMake build regenerates the snapshot when the DIPfile or any
of its input files changes; use the ``parameter-viewer-snapshot`` target to
regenerate it by itself.
The generated snapshot records evaluation dependencies and stores source
paths relative to its own directory, so the example can be moved as a unit.

Inspect the ``DIPfile`` branch to see the project's registered inputs and
their manifest lines. The structured manifest branch is available when
opening the live DIPfile; the snapshot contains evaluated results.

In the browser, compare ``experiment.geometry`` (group),
``experiment.materials`` (map), ``experiment.probes`` (list),
``experiment.duration`` (value), and ``experiment.repeats`` (value with a
child). Select ``experiment.geometry.length`` to see its declaration and
override. The ``Overridden nodes`` branch lists this target separately and links
back to its evaluated path. Then select ``experiment.average_speed`` to follow
its dependencies.
The effective values are ``3 m`` and ``0.375 m/s`` respectively. The
``Schemas`` branch lists ``probe`` with its declaration and links to values it
contributed; ``Units`` lists the ``sample_tick`` custom unit and its definition.
``Named sources`` contains the separate ``reference`` and ``catalog``
sources. Select ``experiment.lab_name`` and follow its
``reference?lab_name`` dependency to inspect the source value and declaration.
Select ``experiment.instrument_family`` to navigate into the catalog's map.
The ``raw_samples`` source opens as plain text and supplies
``experiment.sample_values`` through raw source injection.
The inline ``experiment.readings`` table and ``experiment.sample_grid`` array
are browsable under ``Block value sources``. Raw file inputs have their own
``Raw named sources`` branch.
The ``table_samples`` source supplies ``experiment.reference_readings`` and
shows its column header with DIPL highlighting above the raw data rows.
Source browsing requires a live
project parse; source nodes are not yet stored in ``.diph5`` snapshots. The
`example README
<https://github.com/scinumtools/snt3/blob/main/examples/dip/ParameterViewer/README.md>`_
provides a longer guided tour and a command-line parse check.

Adapter Outputs
---------------

The `AdapterOutputs source directory
<https://github.com/scinumtools/snt3/tree/main/examples/dip/AdapterOutputs>`_
uses ``steps = 4`` and ``dt = 0.25 s`` to demonstrate two independent formats.
The C++ adapter writes a solver namelist, binary marker, streamed time values,
and a DIPH5 snapshot. The Python adapter writes JSON and CSV for an analysis
program. Each adapter registers multiple files in one ``plan()`` call.

After building ``ExampleDipAdapter`` and ``_snt``, run them from the repository
root with fresh output directories:

.. code-block:: console

   build/bin/ExampleDipAdapter examples/dip/AdapterOutputs/DIPfile build/adapter-cpp
   PYTHONPATH=build/python python examples/dip/AdapterOutputs/convert.py --output build/adapter-python

The first command creates ``solver/control.nml``, ``solver/magic.bin``,
``solver/times.dat``, and ``run.diph5``. The second creates
``analysis/job.json`` and ``analysis/times.csv``. See :doc:`the adapter guide
<../modules/dip/adapters>` for the API and path rules.

.. _dip-create-report-example:

Create Report
-------------

The `CreateReport source directory <https://github.com/scinumtools/snt3/tree/main/examples/dip/CreateReport>`_
contains a DIPfile project with two schemas, two custom units, multiple
source files, and an override. Its LaTeX introduction and cover fields show
how to add context to the generated report. The
:download:`example PDF <../../../examples/dip/CreateReport/report.pdf>` is
included in the repository for immediate viewing.

The PDF contents page links to an ``Override example`` subsection. The
Parameter guide links effective values to their full entries. The Parameters
section shows the effective ``experiment.flow_speed`` value of
``3 m/s`` alongside its ``2 m/s`` declaration and ``overrides.dip:1`` source.
The derived ``experiment.flow_distance`` entry shows the speed and duration
values read during evaluation. The Schemas section lists the parameters each
schema supplied. The report also lists custom units and available publication
references. From the
repository root, generate a TeX file without external tools:

.. code-block:: bash

   build/bin/snt report --project examples/dip/CreateReport/DIPfile \
       --intro examples/dip/CreateReport/introduction.tex \
       --output build/create-report.tex

For a PDF, add ``--format pdf`` and choose a ``.pdf`` output path; this
requires a local TeX compiler. See :doc:`../integrations/cli` for the
complete PDF command and report options.

Generated report
^^^^^^^^^^^^^^^^

The bundled PDF shows the cover, linked contents, and styled sections produced
by the example. The `PDF can also be opened separately
<../_static/create-report.pdf>`_.

.. raw:: html

   <iframe
       src="../_static/create-report.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
