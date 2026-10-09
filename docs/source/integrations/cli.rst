CLI Command-line interface
==========================

The ``snt`` executable evaluates PUEL expressions, converts quantities, and
parses DIPL definitions from a terminal or shell script. It uses the same
C++ implementation as the language bindings.

Getting started
---------------

For a source build, enable ``ENABLE_EXEC_APPS`` and
``ENABLE_EXEC_APPS_SNT``, together with the PUQ, DIP, and API modules.
These options are enabled by default. The executable is written to
``build/bin/snt`` and installed into the installation prefix's ``bin``
directory. See :doc:`../installation` for build and installation steps.

Once the executable is on your ``PATH``, inspect the available commands:

.. code-block:: bash

   snt -h
   snt puq -h
   snt dip -h
   snt report -h

Quantities and units
--------------------

The PUQ commands evaluate expressions, convert units, inspect quantities,
and list available unit definitions. Quote expressions so the shell passes
them as a single argument:

.. code-block:: bash

   snt puq eval "23*cm + 3*m"
   snt puq convert "2.5*m" "cm"
   snt puq info "23*kg*m2/s2"
   snt puq list deriv

For conversions between unit systems, specify the input system with ``-s``
and output system with ``-S``. Some conversions also require the physical
quantity, supplied with ``-Q``:

.. code-block:: bash

   snt puq convert "12*statA" "A" -s ESU -S SI -Q "I"

DIPL parameters
---------------

Compare saved environments with ``snt dip compare``. It prints a summary and
up to 50 differences by default. Exit status is 0 when equal in the selected
scope, 1 when different, and 2 for invalid input or an error.

.. code-block:: bash

   snt dip compare before.diph5 after.diph5
   snt dip compare before.diph5 after.diph5 --scope full --max-details 20

``--max-array-examples N`` controls the number of flat array indices shown
for each changed array. See :ref:`DIPH5 comparison <dip-diph5-comparison>`
for scope semantics.

Use ``dip parse`` with ``--input file`` to load a file or ``--input string``
to supply definitions directly. ``--print`` displays nodes with names and
units, and ``--request`` selects a node:

.. code-block:: bash

   snt dip parse --input file parameters.dip --print
   snt dip parse --input string "answer int = 42" --request answer --print

Repeat ``--input`` to combine sources before evaluating a request.

For a reusable set of units, named sources, DIPL files, and inline DIPL,
place the declarations in a :doc:`DIPfile project <../modules/dip/projects>`:

.. code-block:: bash

   snt dip parse --project DIPfile --request simulation.steps --print

``--project`` supplies the model and resolves paths relative to the DIPfile.
It can be combined with ``--input override_string`` or ``--input override_file``
to tune its values.

Values for shell scripts
------------------------

The ``--value`` option prints exactly one defined, unitless scalar without
its name or string quotes. It requires ``--request`` and cannot be combined
with ``--print``. Optionally use ``--type`` to require ``bool``, ``integer``,
``float``, or ``string`` without implicit conversion:

.. code-block:: bash

   answer=$(snt dip parse --input string "answer int = 42" \
       --request answer --value --type integer)
   printf '%s\n' "$answer"

In this mode, invalid requests write an error to standard error and return
a nonzero exit status. Arrays, values with units, and undefined values are
rejected. For reading such scalar settings during CMake configuration, see
:doc:`cmake`.

Registering schemas
-------------------

Schema inputs take a name and either body text or a file containing the body,
without a ``$schema`` wrapper:

.. code-block:: sh

   snt dip parse -i schema_string settings 'value int = 42' \
       -i string 'physics : settings' -r physics.value --value
   snt dip parse -i schema_file settings settings.dipl \
       -i string 'physics : settings' --print

As with other individual inputs, these cannot be combined with ``--project``
or ``--load``.

Overriding values
-----------------

Pass an unwrapped override body using ``override_string``:

.. code-block:: shell

   snt dip parse -i file parameters.dip \
       -i override_string 'simulation.steps = 1024' --print

Duplicate targets fail, including duplicates in ``$override`` regions in files.
Project inputs accept override text alongside the manifest:

.. code-block:: shell

   snt dip parse --project DIPfile \
       -i override_string 'simulation.steps = 1024' --print

The project and override inputs may be supplied in either order.

Use ``-i override_file overrides.dip`` to read an unwrapped override body from
a file.

.. _cli-semantic-interface:

Describing and previewing parameters
------------------------------------

The read-only semantic commands return versioned JSON. ``describe`` reports
one evaluated path, while ``list`` selects values by path and explicit tags:

.. code-block:: shell

   snt dip describe --project DIPfile --path simulation.steps --format json
   snt dip list --project DIPfile --query '?simulation.' --limit 50 --format json
   snt dip list --project DIPfile --tag-all runtime --format json
   snt dip override-contract --project DIPfile --path simulation.steps --format json

Use ``--input`` for a DIPL file or a DIPH5 snapshot. Values in the JSON
response have a DIPL text encoding. Values exceeding ``--max-value-elements``
or the 4096-byte text cap are omitted, and tables have no single value.
``list`` omits value content by default and
reports whether its result was truncated. Missing defaults and dependency
graphs are marked unavailable instead of being inferred. Add
``--record-graph`` to ``describe`` or ``list`` to capture dependencies while
parsing a DIPfile or DIPL input. A DIPH5 snapshot uses its saved graph state.

``override-contract`` classifies an override target as ``existing_value``,
``existing_item``, ``new_item``, or ``unavailable``. It reports applicable
constraints and a reason for unavailable targets. A new collection item needs
a schema on its collection; append a list item with ``items[]`` or create a
map item with ``items[key]``. This is a static guide: use ``preview`` to check
the proposed value and dependent calculations.

For a populated example, run this from the repository root:

.. code-block:: shell

   snt dip describe --input examples/dip/SemanticDescription/parameters.dip \
       --path simulation.speed --record-graph --format json

It reports the evaluated speed, units, descriptive metadata, tags, enforced
condition, source location, and reads of ``simulation.distance`` and
``simulation.duration``. Metadata fields absent from the DIPL input remain
empty; the tool does not invent publication details or a default value.

``preview`` evaluates the original model and a separate candidate with
ordered overrides. It reports validation diagnostics and an effective-value
diff without writing project files or generated outputs:

.. code-block:: shell

   snt dip preview --project DIPfile --override-file candidate.dip --format json
   snt dip preview --input parameters.dip --override 'simulation.steps = 200' \
       --max-details 20 --format json

An invalid candidate is a JSON result with ``candidate_valid: false`` and
``diff.available: false``. The command exits successfully when it can return
that result; command-line usage errors return a nonzero status. Preview
accepts DIPfile, ``.dip``, and ``.dipl`` inputs, while description and listing
also accept DIPH5 snapshots.

Environment persistence
-----------------------

Use ``--save`` to write the full evaluated environment to a DIPH5 file.
Use ``--load`` instead of ``--input`` to query saved parameters without
reevaluating their DIPL sources:

.. code-block:: bash

   snt dip parse --input string "simulation.steps int = 100" --save parameters.diph5
   snt dip parse --load parameters.diph5 --print
   snt dip parse --load parameters.diph5 --request simulation.steps --value --type integer
   snt dip parse --load parameters.diph5 --save copy.diph5
   snt dip parse --project DIPfile --record-graph --relative-source-paths \
       --save project.diph5

``--load`` cannot be combined with any ``--input``. ``--save`` overwrites an
existing file and does not require ``--print`` or ``--value``. It saves the
entire environment even when ``--request`` or ``--tags`` restricts printed
output. If output validation fails, the destination is not written.
``--record-graph`` records evaluation dependencies while parsing DIPL input;
the graph is stored by ``--save``. It cannot be used with ``--load``.
``--relative-source-paths`` stores source-manifest paths relative to the saved
DIPH5 file and requires ``--save``. This is useful when a project and its
snapshot are distributed together.
File errors are reported on standard error with a nonzero exit status.
See :doc:`DIP environment persistence <../modules/dip/persistence>` for
the format and its limitations.

Generating static parameters
----------------------------

Use ``--generate <format> <file>`` to export the complete evaluated
environment as language-native source or a data file. It works after parsing
DIPL input or loading a DIPH5 environment:

.. code-block:: bash

   snt dip parse --input file parameters.dip --generate cpp parameters.hpp
   snt dip parse --load parameters.diph5 --generate julia parameters.jl
   snt dip parse --input file parameters.dip --generate json parameters.json

Supported format names are ``cpp``, ``c``, ``fortran``, ``rust``, ``julia``,
``json``, and ``yaml``. ``--generate`` can accompany ``--save``. Requests and
tags only affect text printed by the command, not the generated environment.
See :doc:`Static parameter generation <../modules/dip/generation>` for the
native representations and format-specific behavior.

Generating reports
------------------

``snt report`` writes a report of evaluated values and units, descriptions,
parameter paths, source identities, custom unit definitions, schema information,
override status, and available publication metadata. Function names are listed
when registered; executable function bodies are not included. Built-in PUQ unit
catalogues are not duplicated in the report. Output paths are sorted for repeatable reports. The
default format is ``tex``; it requires no external tools:

.. code-block:: bash

   snt report --project DIPfile --output report.tex
   snt report --load run.diph5 --output report.tex
   snt report --input file parameters.dip --output report.tex

Brief++ also renders the same report as Markdown (``md``), reStructuredText
(``rst``), HTML (``html``), Typst (``typ``), plain text (``txt``), or a
``briefpp/1`` document tree (``json``):

.. code-block:: bash

   snt report --project DIPfile --format html --output report.html
   snt report --project DIPfile --format md --output report.md
   snt report --load run.diph5 --format json --output report.json

These formats need no external tools. Markdown tables use Brief++'s
MyST-style directives. Report JSON describes the document; use
``snt dip parse --generate json`` for static parameter export JSON.

The command accepts the same ``--input`` kinds as ``snt dip parse``. A project
may be combined with ``override_string`` or ``override_file`` inputs. A DIPH5
``--load`` cannot be combined with other inputs. For loaded snapshots, the
report contains only provenance retained in DIPH5.
Reports from DIPL inputs record evaluation dependencies automatically. The
Parameter guide links effective values to full reference entries, and the
Schemas section lists the parameters each schema supplied. A loaded DIPH5
report includes calculation relationships only if its snapshot contains a
recorded graph; otherwise the report states that they are unavailable.
Empty groups and collections without value descendants are not present in a
DIPH5 snapshot and therefore cannot appear in a report generated from it.

Use ``--intro introduction.tex`` to insert trusted LaTeX after the report
contents page. The file is read as a fragment, without a document preamble. Its text
is included in both TeX and PDF reports. Other formats reject ``--intro``.

``--format pdf`` compiles the same generated TeX with ``pdflatex``. Select a
compatible executable with ``--tex-compiler`` when needed:

.. code-block:: bash

   snt report --project DIPfile --format pdf --output report.pdf
   snt report --load run.diph5 --format pdf --output report.pdf \
       --tex-compiler /path/to/pdflatex

PDF output requires a locally installed TeX compiler. If it is unavailable or
compilation fails, the command reports an error and does not create the output
PDF. The TeX compiler is never installed by ``snt report``.
For reports containing Unicode characters unsupported by ``pdflatex``, use
``--tex-compiler lualatex`` when that compiler is installed.
Use ``--title``, ``--author``, ``--date``, and ``--report-version`` for the
cover. To regenerate the bundled :ref:`CreateReport example
<dip-create-report-example>` PDF from the repository root:

.. code-block:: bash

   build/bin/snt report --project examples/dip/CreateReport/DIPfile \
       --intro examples/dip/CreateReport/introduction.tex \
       --title "Mock Heat Flow Study" --author "Example Research Team" \
       --date "2026-09-28" --report-version "1.0 demo" \
       --format pdf --output examples/dip/CreateReport/report.pdf

The default cover title is ``DIP parameter report``; an empty author appears
as ``Not specified``. The date defaults to the local generation date and the
version to the SNT build version. Set date and version explicitly for a
reproducible cover. The :ref:`CreateReport example
<dip-create-report-example>` shows the layout and provides a PDF to inspect.

Server, dmap, and viewer commands
---------------------------------

The main executable also hosts the REST server and dimension-map generator:

.. code-block:: sh

   snt server --help
   snt server --port 8081
   snt server --project model=/srv/model/DIPfile
   snt dmap --help

Build server support with ``ENABLE_SNT_SERVER=ON`` and dimension-map
generation with ``ENABLE_SNT_DMAP=ON``. Server support requires cpp-httplib;
builds without it can set ``ENABLE_SNT_SERVER=OFF``. See :doc:`rest` for routes
and options.

``snt dmap`` is a developer tool for precomputing PUQ unit dimensions. Run it
from the source repository root only when updating unit definitions: it
overwrites ``src/snt/puq/systems/dmaps/dmap_*.h``. Review the generated diff
before committing. Its ``-e`` option replaces those headers with empty
placeholders.

The optional ``snt view <artifact>`` command opens the graphical
:doc:`Parameter Viewer <viewer>`. Its guide covers building the viewer,
browsing live projects and DIPH5 snapshots, opening source locations, and
reloading after an edit.

All three features belong to the ``snt`` target and require ``ENABLE_EXEC_APPS_SNT``.
