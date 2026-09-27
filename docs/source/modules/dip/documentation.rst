Generating DIP documentation
============================

``snt docs`` turns an evaluated DIP environment into a TeX report or a PDF.
The report lists effective values and units, descriptions, hierarchy paths,
schema information, override status, source identities, custom unit definitions,
registered function names, and available publication metadata. Entries are
sorted by path or name so repeated runs have a stable order. Generated text is
escaped for LaTeX.

Run the ``examples/dip/CreateDocs`` demo from the repository root to try a
DIPfile with a schema, an override, and a custom unit:

.. code-block:: bash

   build/bin/snt docs --project examples/dip/CreateDocs/DIPfile \
       --intro examples/dip/CreateDocs/introduction.tex \
       --output build/create-docs-report.tex

``--format tex`` is the default and needs no external program. To produce a
PDF, add ``--format pdf`` and an output path ending in ``.pdf``. This invokes
``pdflatex`` by default; ``--tex-compiler`` selects another compatible local
TeX executable. If the compiler is missing or fails, the command reports an
error and does not create a new PDF.

The optional ``--intro`` file contains a trusted LaTeX fragment, without
``\documentclass`` or a document preamble. It appears after the report title
in both TeX and PDF output. For Unicode text unsupported by ``pdflatex``, use
``--tex-compiler lualatex`` when available.

The same renderer is available to C++, Python, and C applications from an
already evaluated :cpp:class:`snt::dip::Environment`:

.. code-block:: cpp

   #include <snt/dip/dip.h>
   #include <snt/docs/report.h>

   snt::dip::DIP parser;
   parser.add_project("DIPfile");
   auto environment = parser.parse();
   snt::docs::ReportOptions options;
   options.input_label = "DIPfile";
   options.introduction_file = "introduction.tex";
   snt::docs::generate(environment, snt::docs::ReportFormat::Tex,
                       "report.tex", options);

See :doc:`the Python binding <../../integrations/python>` and
:doc:`the C binding <../../integrations/c>` for their corresponding calls.
The C++ declaration is in :doc:`the docs API reference <../../api/cpp_docs>`.

DIPH5 snapshots contain evaluated values and retained provenance. A report
generated from a loaded snapshot cannot reconstruct original source text,
executable function bodies, reusable schema definitions, or empty containers
that were not saved. Built-in PUQ unit catalogues are not duplicated in the
report; custom units registered in the environment are listed.
