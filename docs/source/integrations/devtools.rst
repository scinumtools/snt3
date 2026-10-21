DIPL developer tools
====================

This page covers tools for maintaining DIPL source. The formatter is available
now; validator and linter commands can be documented here as they are added.

Formatter
---------

``snt fmt`` formats DIPL source files with the native C++ formatter. It puts
one blank line between sibling node blocks, keeps ``?`` metadata and ``!``
directives with their node, spaces assignments and inline array commas, and
ends the file with a newline. It preserves values, comments, strings, array
order, multiline scalar contents, and CRLF line endings. Indentation is fixed
at two spaces per level by DIPL and is not a style setting.

.. code-block:: bash

   snt fmt parameters.dip             # print formatted source
   snt fmt --check parameters.dip     # exit 1 if formatting is needed
   snt fmt --write parameters.dip     # update the file
   snt fmt --check a.dip b.dip        # check several files

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
