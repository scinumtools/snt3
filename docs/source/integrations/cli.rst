Command-line interface
======================

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

DIPL definitions
----------------

Use ``dip parse`` with ``--input file`` to load a file or ``--input string``
to supply definitions directly. ``--print`` displays nodes with names and
units, and ``--request`` selects a node:

.. code-block:: bash

   snt dip parse --input file parameters.dip --print
   snt dip parse --input string "answer int = 42" --request answer --print

Repeat ``--input`` to combine sources before evaluating a request.

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
