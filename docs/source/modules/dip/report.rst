Report generation
=================

``snt::dip::report::generate`` writes a report from an evaluated
``snt::dip::Environment``. The application-facing
``snt::api::generate_dip_report`` calls the same generator. Both use the
document model in the DIP library. The renderer is `Brief++
<https://github.com/vrtulka23/briefpp>`_, a header-only library. Supported
formats are TeX, PDF, Markdown, reStructuredText, HTML, Typst, plain text,
and Brief++ document JSON. All contain the evaluated report data; TeX and
PDF additionally use the styled cover and contents page.

Generating a report
-------------------

Parse a DIPfile project and pass the resulting environment to the API:

.. code-block:: cpp

   #include <snt/api/dip_report.h>
   #include <snt/dip/dip.h>

   snt::dip::DIP parser;
   parser.add_project("DIPfile");
   auto env = parser.parse();

   snt::dip::report::ReportOptions options;
   options.input_label = "DIPfile";
   options.introduction_file = "introduction.tex";
   snt::api::generate_dip_report(
       env, snt::dip::report::ReportFormat::Tex, "report.tex", options);

TeX output needs no external program. ``introduction_file`` is an optional
trusted LaTeX fragment for TeX and PDF only, without a document preamble. It appears after the
contents page; ``\subsection`` headings in the fragment become contents
links. Other text supplied by the environment and cover options is escaped
for LaTeX.

PDF output uses the same TeX document and invokes a local TeX compiler twice
to resolve contents links and page numbers:

.. code-block:: cpp

   options.title = "Mock Heat Flow Study";
   options.author = "Example Research Team";
   options.date = "2026-09-28";
   options.version = "1.0 demo";
   options.tex_compiler = "pdflatex";
   snt::api::generate_dip_report(
       env, snt::dip::report::ReportFormat::Pdf, "report.pdf", options);

``tex_compiler`` defaults to ``pdflatex``. Choose another compatible local
compiler, such as ``lualatex`` for Unicode text, when needed. An unavailable
or failing compiler raises an error; the generator does not install one.

Other formats use Brief++ renderers directly and need no external program:

.. code-block:: cpp

   options.introduction_file.clear();
   snt::api::generate_dip_report(
       env, snt::dip::report::ReportFormat::Html, "report.html", options);
   snt::api::generate_dip_report(
       env, snt::dip::report::ReportFormat::Markdown, "report.md", options);

The other format values are ``Rst``, ``Typst``, ``Text``, and ``Json``.
Markdown uses Brief++'s MyST-style directives for tables. JSON is the
``briefpp/1`` document tree, not DIP's static parameter export JSON.
An introduction file is rejected for these formats because it contains raw
LaTeX.

Report options and content
--------------------------

``ReportOptions`` defaults to the title ``DIP parameter report``, an
unspecified author, the local generation date, and the SNT build version.
Set the date and version explicitly when a reproducible cover matters.
``input_label`` identifies the environment on the cover. Set
``source_root`` to a project directory to show source paths relative to it
when possible.

The report contains effective values, types, array shapes and units,
descriptions, parameter paths, schema information, source provenance, custom
units, registered function names, and available publication references.
Applied value modifications appear in order with their source locations.
Override information appears only for overridden parameters. Evaluated tables
have a summary of their row count and ordered column names, types, and units;
their column values remain in the Parameters section. Entries are sorted by
path or name. The PDF has a cover, linked contents page, and matching shaded
headings for parameters, tables, schemas, sources, and custom units.
Human-readable reports show a 16-character prefix of each source content hash
for readability. The JSON report, environment, and DIPH5 snapshot retain full
digests.
Parameter paths retain their group names; there is no separate hierarchy
section. Built-in PUQ unit catalogues and executable function bodies are not
included.

The generator also accepts a loaded DIPH5 environment:

.. code-block:: cpp

   #include <snt/dip/environment.h>
   #include <snt/dip/report/report.h>

   snt::dip::Environment env;
   env.load("run.diph5");
   snt::dip::report::generate(
       env, snt::dip::report::ReportFormat::Tex, "loaded-report.tex");

This report contains only values and provenance retained in the snapshot.
It cannot reconstruct unsaved source text, executable function bodies,
reusable schema definitions, or empty containers without value descendants.

See the :doc:`DIP report declarations <../../api/cpp/dip/report>` for the full
options and the :ref:`API wrapper declaration <cpp-api-report-generation>`.
The :ref:`CreateReport example <dip-create-report-example>` includes
a generated PDF. For other interfaces, see the :doc:`CLI
<../../integrations/cli>`, :doc:`Python <../../integrations/python>`,
:doc:`C <../../integrations/c>`, and :doc:`REST
<../../integrations/rest>` guides.
