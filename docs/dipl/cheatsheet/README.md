# DIPL & PUEL quick reference

`cheatsheet.md` is the canonical text; `template.tex` controls the printable
layout. The checked-in `dipl-cheatsheet.pdf` is the prebuilt distribution
resource used by the DIPL specification, Sphinx, and installed packages.

## Build

Install Pandoc, LuaLaTeX (including `fontspec`, `multicol`, `tcolorbox`,
`listings`, and TeX Gyre fonts), Ghostscript, and a Python installation of
SNT3 with its compiled DIPL/PUEL bindings. From the repository root:

```sh
make -C docs pdf-reference
```

`build.py` resolves the DIPL, PUEL, DIPH5, and SNT3 versions from the
specifications and repository settings, checks the DIPH5 writer version,
verifies marked examples with the actual parsers, then builds the PDF. It
rejects PDFs over two pages or missing the version labels. The resulting PDF
is copied only when its inputs change. The specification includes it as the
last appendix with `pdfpages`.

`make -C docs html` builds the flyer and all three language specification
PDFs. Each specification PDF is copied into its corresponding `docs/`
directory and into Sphinx's static assets. The standalone flyer is published
at `dipl-cheatsheet.pdf` beside the Sphinx HTML output.

## Verified examples

Use Pandoc fenced-code attributes in `cheatsheet.md`:

- `.verify`: complete DIPL model or PUEL expression, parsed and validated.
- `.fragment`: DIPL fragment wrapped in a temporary group before validation.
- `.illustrative`: syntax excerpt that is not automatically checked.
- `.invalid`: a complete example expected to fail validation.

Run `python docs/dipl/cheatsheet/verify.py` to check the marked examples
without invoking Pandoc or LaTeX.

## Application reports

In Python, call `environment.generate_report(ReportFormat.PDF, "report.pdf",
include_cheatsheet=True)` to append the packaged PDF after the application's
parameter report. The default remains `False`. The appendix is read locally
from the installed SNT3 package. The existing report PDF renderer still uses
a local TeX compiler for the model-specific part of the report.

In C++, set `ReportOptions::cheatsheet_pdf` to the installed prebuilt PDF path
before calling `snt::dip::report::generate`. CMake installations place the
resource under `share/snt3/dipl-cheatsheet.pdf` (or the platform's configured
data directory).
