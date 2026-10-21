DIPL developer tools
====================

This page covers tools for maintaining DIPL source. Try the formatter,
validator, and linter with the :ref:`dip-devtools-example`.

Formatter
---------

``snt format`` formats DIPL source files with the native C++ formatter. It puts
one blank line between sibling node blocks, keeps ``?`` metadata and ``!``
directives with their node, spaces assignments and inline array commas, and
ends the file with a newline. It preserves values, comments, strings, array
order, multiline scalar contents, and CRLF line endings. Indentation is fixed
at two spaces per level by DIPL and is not a style setting.

.. code-block:: bash

   snt format parameters.dip             # print formatted source
   snt format --check parameters.dip     # exit 1 if formatting is needed
   snt format --write parameters.dip     # update the file
   snt format --check a.dip b.dip        # check several files

``--check`` exits 0 when all inputs match and 2 for an error. ``--write``
accepts several files. The formatter reads source text directly; it does not
evaluate the model or resolve external data files.

Formatter settings in DIPfile
-----------------------------

The nearest parent ``DIPfile`` supplies formatter settings. Add the built-in
``format`` group to override its typed defaults:

.. code-block:: dipl

   format
     blank_lines_between_nodes = 0
     final_newline = true

The default number of blank lines is 1; the allowed range is 0 to 4. The
``final_newline`` default is ``true``. The same settings apply when formatting
the DIPfile itself. Standalone files without a parent DIPfile use the defaults.
The formatter leaves existing indentation in place because malformed nesting
cannot always be inferred safely from whitespace alone.

The C++ entry points are ``snt::dip::tools::format(source, options)`` in
``<snt/dip/tools/formatter.h>`` and the file-level helper
``snt::api::format_dip_file(path)`` in ``<snt/api/dip_format.h>``.

Validator
---------

``snt validate`` uses the native C++ DIPL parser and evaluator to check a
source file or a DIPfile project. A file named ``DIPfile`` is recognized as a
project automatically; ``--project`` also accepts a manifest with another
filename. Validation reads project dependencies but does not write output
files.

.. code-block:: bash

   snt validate parameters.dip
   snt validate DIPfile
   snt validate --project experiment.manifest
   snt validate --json parameters.dip
   snt validate a.dip b.dip

Exit status is 0 for valid input, 1 for invalid DIPL, and 2 for a missing or
unreadable input or command error. The validator continues across separate
files and reports multiple independent statement syntax errors in source files,
DIPfile manifests, and project code files. It then skips semantic evaluation
of incomplete syntax. By default, at most 100 syntax errors are reported per
input.
Semantic errors and project schema or override registration errors remain
fail-fast; an unclosed multiline construct may prevent recovery beyond that
point. The JSON result includes each diagnostic's code, message, details,
suggestion, and source location. With
several inputs, ``--json`` returns an array of results.

The C++ entry points are ``snt::dip::tools::validate_text()`` and
``validate_file()`` in ``<snt/dip/tools/validator.h>``. The application API
provides ``snt::api::validate_dip_file()`` and ``dip_validation_json()`` in
``<snt/api/dip_validate.h>``.

Validator settings in DIPfile
-----------------------------

The nearest parent ``DIPfile`` supplies the diagnostic limit for source files.
When validating project code, that project's manifest supplies it. A manifest
with syntax errors uses the default limit because its settings cannot yet be
evaluated. The built-in ``validate`` group has one setting:

.. code-block:: dipl

   validate
     max_syntax_errors = 25

``max_syntax_errors`` defaults to 100 and accepts values from 1 to 1000. It
limits how many independent syntax errors are shown; it does not change
whether a file is valid or how semantic rules are checked. Files without a
parent DIPfile use the default. The C++ ``validate_text()`` function accepts
the same limit through ``ValidationOptions``.

Linter
------

``snt lint`` first validates and evaluates DIPL, then reports advisory issues
with source locations. Invalid input reports validation errors only. The
default rules warn when an explicitly public value or schema lacks ``?descr``,
when an evaluated value carries ``?deprecated``, when ``!options`` contains
an exact duplicate, and when a public numeric choice list lacks complete
``!labels``. Deprecation findings use ``?replacement`` in their suggestion
when present. These rules do not change whether a model is valid.

Two optional rules can check local paths named by ``?replacement``,
``?requires``, ``?conflicts``, and ``?implies``, and schemas with no evaluated
application. The reference check only treats a ``?``-prefixed or dotted or
collection path as a local path; plain words and prose are skipped. It checks
the evaluated model, so conditional paths absent from that model may be
reported. Unused schemas can be intentional in a reusable schema library.

.. code-block:: bash

   snt lint parameters.dip
   snt lint DIPfile
   snt lint --project experiment.manifest
   snt lint --json parameters.dip

Exit status is 0 when clean, 1 for lint findings or invalid DIPL, and 2 for an
unreadable input or command error. JSON contains ``valid`` for input validity,
``clean`` for lint cleanliness, and structured diagnostics with severity,
source location, and node path. Multiple inputs produce an array.

The nearest parent ``DIPfile`` supplies the ``lint`` settings for standalone
files; a project uses its own manifest. The first four rules default to enabled,
and the last two are opt-in:

.. code-block:: dipl

   lint
     require_public_descriptions = true
     warn_deprecated_values = true
     warn_duplicate_options = true
     require_public_option_labels = true
     warn_broken_metadata_references = false
     warn_unused_schemas = false

The C++ entry points are ``snt::dip::tools::lint_environment()``,
``lint_text()``, and ``lint_file()`` in ``<snt/dip/tools/linter.h>``. The
application API exposes ``snt::api::lint_dip_file()`` and ``dip_lint_json()``
in ``<snt/api/dip_lint.h>``.
