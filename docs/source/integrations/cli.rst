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

Environment persistence
-----------------------

Use ``--save`` to write the full evaluated environment to a DIPH5 file.
Use ``--load`` instead of ``--input`` to query saved parameters without
reevaluating their DIPL sources:

.. code-block:: bash

   snt dip parse --input string "simulation.steps int = 100" --save parameters.diph5
   snt dip parse --load parameters.diph5 --print
   snt dip parse --load parameters.diph5 --request simulation.steps --value --type integer
   snt dip parse --load parameters.diph5 --save copy.diph5

``--load`` cannot be combined with any ``--input``. ``--save`` overwrites an
existing file and does not require ``--print`` or ``--value``. It saves the
entire environment even when ``--request`` or ``--tags`` restricts printed
output. If output validation fails, the destination is not written.
File errors are reported on standard error with a nonzero exit status.
See :doc:`DIP environment persistence <../modules/dip/persistence>` for
the format and its limitations.

Generating static parameters
----------------------------

Use ``--generate <format> <file>`` to export the complete evaluated
environment as language-native source or a data file. It works after parsing
DIPL input or loading a DIPH5 environment:

.. code-block:: bash

   snt dip parse --input file parameters.dip --generate cpp parameters.hpp
   snt dip parse --load parameters.diph5 --generate julia parameters.jl
   snt dip parse --input file parameters.dip --generate json parameters.json

Supported format names are ``cpp``, ``c``, ``fortran``, ``rust``, ``julia``,
``json``, and ``yaml``. ``--generate`` can accompany ``--save``. Requests and
tags only affect text printed by the command, not the generated environment.
See :doc:`Static parameter generation <../modules/dip/generation>` for the
native representations and format-specific behavior.

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
