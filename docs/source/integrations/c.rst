C bindings
==========

.. note::

   The C bindings are experimental and still under development. The interface
   is incomplete, and function signatures and behavior may change in future
   releases.

The C binding exposes PUQ quantities through ``<snt/c/puq.h>`` and DIPL parsing
through ``<snt/c/dip.h>``. It uses opaque handles: C applications hold pointers to SNT objects and
operate on them through functions, while the implementation remains in C++.

Building and linking
--------------------

Build and install SciNumTools with ``ENABLE_BINDING_C=ON`` (the default)
and the PUQ and DIP modules enabled. See :doc:`../installation` for the
source build instructions. In a consuming CMake project, link to ``snt-c``:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.22)
   project(snt_c_example LANGUAGES C CXX)

   find_package(snt CONFIG REQUIRED)
   add_executable(snt-c-example main.c)
   target_link_libraries(snt-c-example PRIVATE snt-c)
   set_target_properties(snt-c-example PROPERTIES LINKER_LANGUAGE CXX)

The C++ linker supplies the runtime needed by the underlying C++ libraries;
the application source can remain C.

Quantities and unit conversion
------------------------------

Use ``snt_puq_quantity_eval`` to evaluate a PUEL expression,
``snt_puq_quantity_convert`` to create a quantity in the requested units, and
``snt_puq_quantity_format`` to write its textual representation into a buffer.
This complete example converts metres to centimetres:

.. literalinclude:: ../../../examples/puq/CBinding/main.c
   :language: c

DIPL parameters
---------------

Create a parser with ``snt_dip_parser_create``, add definitions with
``snt_dip_parser_add_string`` or ``snt_dip_parser_add_file``, and call
``snt_dip_parser_parse``. Then retrieve a value as text with
``snt_dip_parser_get``. Pass a node path such as
``answer`` or ``project.name`` without a leading ``?``:

.. literalinclude:: ../../../examples/dip/CBinding/main.c
   :language: c

To load a DIPfile manifest, replace the individual input calls with
``snt_dip_parser_add_project(dip, "DIPfile", &error)`` before parsing. See
:doc:`DIPfile projects <../modules/dip/projects>` for the format.

Persisting DIP environments
----------------------------

Use ``snt_dip_environment_save`` and ``snt_dip_environment_load`` with a DIP
handle, a ``const char*`` filename, and an ``snt_dip_error`` object. They
return zero on success, using the same error convention as parsing.
See :doc:`Environment persistence <../modules/dip/persistence>` for the
DIPH5 format and current limitations.

Generating static parameters
----------------------------

After parsing or loading an environment, call
``snt_dip_environment_generate`` with an ``snt_dip_export_format`` and an
output path. It follows the normal C binding error convention:

.. code-block:: c

   snt_dip_error error = {0};
   if (snt_dip_environment_generate(
           dip, SNT_DIP_EXPORT_CPP, "parameters.hpp", &error) != 0) {
       fprintf(stderr, "%s\n", error.message);
   }

The supported C ABI formats are ``SNT_DIP_EXPORT_CPP``,
``SNT_DIP_EXPORT_C``, ``SNT_DIP_EXPORT_FORTRAN``, ``SNT_DIP_EXPORT_RUST``,
``SNT_DIP_EXPORT_JULIA``, ``SNT_DIP_EXPORT_JSON``, and
``SNT_DIP_EXPORT_YAML``. See :doc:`Static parameter generation
<../modules/dip/generation>` for the generated representations.

Errors and ownership
--------------------

Operations returning ``int`` return zero on success and nonzero on error.
Pass an ``snt_puq_error`` or ``snt_dip_error`` to receive the error code and message. The message is
owned by the library; copy it if it must survive a later error on the same
thread.

Release every created quantity with ``snt_puq_quantity_free`` and every parser
with ``snt_dip_parser_free``. Conversion creates a separate quantity, so both the
original and converted handles must be released. The free functions also
accept null pointers.

Output buffers belong to the caller. Their capacity must include space for
the terminating null character; insufficient capacity is reported as an
error.
