DIP examples
============

The DIP examples cover a minimal DIPL definition, CMake configuration,
application adapters, and report generation.

Quick Example
-------------

The ``examples/dip/QuickExample`` directory contains a small DIPL definition
with typed parameters, units, references, and validation properties. It is a
smoke test: the C++ and Python quick-example tests load the README example and
verify that it parses and evaluates successfully.

See the `QuickExample source directory <https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/QuickExample>`_.
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
`CMakeIntegration directory <https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/CMakeIntegration>`_.

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

Adapter Outputs
---------------

The `AdapterOutputs source directory
<https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/AdapterOutputs>`_
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

The `CreateReport source directory <https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/CreateReport>`_
contains a DIPfile project with two schemas, two custom units, multiple
source files, and an override. Its LaTeX introduction and cover fields show
how to add context to the generated report. The
:download:`example PDF <../../../examples/dip/CreateReport/report.pdf>` is
included in the repository for immediate viewing.

The PDF contents page links to an ``Override example`` subsection. The
Parameters section shows the effective ``experiment.flow_speed`` value of
``3 m/s`` alongside its ``2 m/s`` declaration and ``overrides.dip:1`` source.
It also lists the custom
units, schema information, and available publication references. From the
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
