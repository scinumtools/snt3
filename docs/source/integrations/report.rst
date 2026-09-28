TeX and PDF reports
===================

``snt report`` creates a report from an evaluated DIP environment. The PDF has a
cover page, a linked contents page, and sections for parameters, schemas,
sources, custom units, and registered functions when present. Entries in each
section use the same shaded heading style. The report shows
effective values, units, descriptions, overrides, source provenance, and
available publication references. Individual parameter paths retain their
group names; there is no separate hierarchy section.

The same generator powers the CLI, the C++ API, the Python and C bindings, and
the optional REST endpoint. TeX output needs no external program. PDF output
uses a locally installed TeX compiler, ``pdflatex`` by default. The compiler
runs twice so contents links and page numbers are resolved.

Try the example
---------------

The :doc:`CreateReport example <../modules/dip/report>` contains two
schemas, two custom units, multiple source files, an override, and publication
metadata. The :download:`generated example PDF
<../../../examples/dip/CreateReport/report.pdf>` is included for immediate
viewing. To regenerate it from the repository
root:

.. code-block:: bash

   build/bin/snt report --project examples/dip/CreateReport/DIPfile \
       --intro examples/dip/CreateReport/introduction.tex \
       --title "Mock Heat Flow Study" --author "Example Research Team" \
       --date "2026-09-28" --report-version "1.0 demo" \
       --format pdf --output examples/dip/CreateReport/report.pdf

Use ``--format tex --output report.tex`` to inspect or compile the TeX
yourself. ``--tex-compiler lualatex`` can handle Unicode text beyond the
default ``pdflatex`` setup.

Cover fields
------------

``--title``, ``--author``, ``--date``, and ``--report-version`` set the cover
fields. The default title is ``DIP parameter report``. An empty author is
shown as ``Not specified``; the date defaults to the local generation date in
``YYYY-MM-DD`` form; the version defaults to the SNT build version. Supply
``--date`` and ``--report-version`` for a reproducible cover. The input label
comes from the project or DIPH5 filename, or can be set through the library
options. Project source paths are shown relative to the DIPfile directory when
possible. Cover text is escaped for LaTeX.

An optional ``--intro`` file is inserted as trusted LaTeX after the contents
page. It should contain a fragment without a preamble. Introduction headings
made with ``\subsection`` appear as links in the contents page.

Other interfaces
----------------

For C++, call ``snt::api::generate_dip_report(environment, format, output,
options)`` from ``<snt/api/dip_report.h>``. The underlying renderer is also
available as ``snt::dip::report::generate``. See :doc:`the C++ API <../api/cpp_report>`.
Set ``options.source_root`` to shorten source paths when calling the library
directly.

Python exposes ``Environment.generate_report(...)`` with keyword arguments
``title``, ``author``, ``date``, and ``version``. The C binding offers
``snt_dip_environment_generate_report_with_options`` with a
``snt_dip_report_options`` structure. See :doc:`Python <python>` and
:doc:`C <c>`.

The REST server accepts ``POST /snt/dip/report`` with DIPL text or the same
multipart project bundle accepted by ``/snt/dip/parse``. It returns a TeX
attachment by default. Add ``?format=pdf`` for a PDF; the server host must
have ``pdflatex`` installed. The cover fields are query parameters named
``title``, ``author``, ``date``, and ``version``. See :doc:`REST API <rest>`.

DIPH5 reports include only the values and provenance retained in the
snapshot. They cannot reconstruct unsaved source text or executable bodies.
