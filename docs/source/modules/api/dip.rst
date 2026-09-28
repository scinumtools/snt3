.. _module-api-dip:

DIP commands
============

``DIPParse`` accepts DIPL inputs and requests, then executes them once. For
example, parse a file and request one formatted value:

.. code-block:: cpp

   #include <snt/api/dip_parse.h>

   snt::api::DIPParse parse;
   parse.argument_add("file", {"config.dip"});
   parse.argument_request("build.python");
   parse.argument_value("bool");
   std::string value = parse.execute();

For a complete DIPfile manifest, use ``parse.argument_add("project",
{"DIPfile"})``. A project cannot be combined with other DIPL input
arguments; see :doc:`DIPfile projects <../dip/projects>`.

Persisting an evaluated environment
------------------------------------

Use ``argument_save()`` to persist the full evaluated environment during
``execute()``. Use ``argument_load()`` instead of ``argument_add()`` to query
an existing DIPH5 file without reevaluating DIPL:

.. code-block:: cpp

   snt::api::DIPParse save;
   save.argument_add("string", {"simulation.steps int = 100"});
   save.argument_save("parameters.diph5");
   save.execute();

   snt::api::DIPParse load;
   load.argument_load("parameters.diph5");
   load.argument_request("simulation.steps");
   load.argument_value("integer");
   std::string steps = load.execute(); // "100\n"

Loading cannot be combined with DIPL file, string, source, or unit inputs.
Saving can accompany either parsing or loading and overwrites an existing
destination. Request and tag filters affect only text output, not the saved
environment. Output validation must succeed before saving. See
:doc:`DIP environment persistence <../dip/persistence>` for the format and its
limitations.

Generating static parameters
----------------------------

Use ``argument_generate()`` to export static parameters during
``execute()``. Its format is one of ``cpp``, ``c``, ``fortran``, ``rust``,
``julia``, ``json``, or ``yaml``:

.. code-block:: cpp

   snt::api::DIPParse generate;
   generate.argument_add("file", {"parameters.dip"});
   generate.argument_generate("cpp", "parameters.hpp");
   generate.execute();

Generation can follow parsing or DIPH5 loading, and can accompany saving.
It always exports the complete evaluated environment; request and tag filters
only affect text output. See :doc:`Static parameter generation
<../dip/generation>` for the generated representations.

Generating reports
------------------

``snt::api::generate_dip_report`` takes an evaluated environment and writes
a report in TeX, PDF, Markdown, reStructuredText, HTML, Typst, plain text,
or Brief++ document JSON. Parse a DIPfile project, then choose the output format:

.. code-block:: cpp

   #include <snt/api/dip_report.h>
   #include <snt/dip/dip.h>

   snt::dip::DIP parser;
   parser.add_project("DIPfile");
   auto env = parser.parse();

   snt::dip::report::ReportOptions options;
   options.title = "Simulation parameters";
   options.author = "Example Research Team";
   options.input_label = "DIPfile";
   options.introduction_file = "introduction.tex"; // optional LaTeX fragment

   snt::api::generate_dip_report(
       env, snt::dip::report::ReportFormat::Tex, "report.tex", options);

   options.tex_compiler = "pdflatex";
   snt::api::generate_dip_report(
       env, snt::dip::report::ReportFormat::Pdf, "report.pdf", options);

   options.introduction_file.clear(); // Raw LaTeX is only for TeX/PDF.
   snt::api::generate_dip_report(
       env, snt::dip::report::ReportFormat::Html, "report.html", options);

TeX output needs no external tool. PDF output runs the configured local TeX
compiler and reports an error if it is unavailable. The API also accepts an
environment loaded from DIPH5, with only the provenance retained in that
file. See :doc:`C++ report generation <../dip/report>` for cover options,
report content, and a DIPH5 example, or the :ref:`CreateReport example
<dip-create-report-example>` for a generated PDF.

The :doc:`generated DIP command declarations <../../api/cpp/api/dip>` and
:ref:`report declaration <cpp-api-report-generation>` provide the member
reference. API errors use the SNT exception hierarchy; see the
:doc:`exception declarations <../../api/cpp/api/errors>`.
