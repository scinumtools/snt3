Generating DIP reports
======================

``snt report`` turns an evaluated DIP environment into a TeX report or a PDF.
The report lists effective values and units, descriptions, parameter paths,
schema information, override status, source identities, custom unit definitions,
registered function names, and available publication metadata. Entries are
sorted by path or name so repeated runs have a stable order. Generated text is
escaped for LaTeX.
It has a cover page, linked contents page, and no separate hierarchy section.

Run the ``examples/dip/CreateReport`` demo from the repository root to try a
DIPfile with two schemas, an override, and two custom units:

.. code-block:: bash

   build/bin/snt report --project examples/dip/CreateReport/DIPfile \
       --intro examples/dip/CreateReport/introduction.tex \
       --output build/create-report.tex

``--format tex`` is the default and needs no external program. To produce a
PDF, add ``--format pdf`` and an output path ending in ``.pdf``. This invokes
``pdflatex`` by default; ``--tex-compiler`` selects another compatible local
TeX executable. If the compiler is missing or fails, the command reports an
error and does not create a new PDF. The compiler runs twice to resolve
contents links and page numbers.

Use ``--title``, ``--author``, ``--date``, and ``--report-version`` to set the
cover. The date defaults to the local generation date, and the version to
the SNT build version. See :doc:`the report integration <../../integrations/report>`
for details and a fuller example.

The optional ``--intro`` file contains a trusted LaTeX fragment, without
``\documentclass`` or a document preamble. It appears after the contents page
in both TeX and PDF output. For Unicode text unsupported by ``pdflatex``, use
``--tex-compiler lualatex`` when available.

The same renderer is available to C++, Python, and C applications from an
already evaluated :cpp:class:`snt::dip::Environment`:

.. code-block:: cpp

   #include <snt/dip/dip.h>
   #include <snt/api/dip_report.h>

   snt::dip::DIP parser;
   parser.add_project("DIPfile");
   auto env = parser.parse();
   snt::dip::report::ReportOptions options;
   options.input_label = "DIPfile";
   options.introduction_file = "introduction.tex";
   snt::api::generate_dip_report(env, snt::dip::report::ReportFormat::Tex,
                               "report.tex", options);

See :doc:`the Python binding <../../integrations/python>` and
:doc:`the C binding <../../integrations/c>` for their corresponding calls.
The C++ declaration is in :doc:`the report API reference <../../api/cpp_report>`.

DIPH5 snapshots contain evaluated values and retained provenance. A report
generated from a loaded snapshot cannot reconstruct original source text,
executable function bodies, reusable schema definitions, or empty containers
that were not saved. Built-in PUQ unit catalogues are not duplicated in the
report; custom units registered in the environment are listed.
