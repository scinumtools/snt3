PUQ — C++ API interface
=======================

The PUQ API provides command-oriented C++ classes for evaluating and
converting physical-unit expressions, inspecting expression information, and
listing available definitions. The commands return formatted text and use the
same PUEL and PUQ implementations as the command-line tool.

Evaluating an expression
------------------------

Create a ``PUQEval`` command with an expression, configure any requested
output, and execute it:

.. code-block:: cpp

   #include "snt/api/puq_eval.h"

   snt::api::PUQEval eval("12 km / 3 h");
   eval.argument_output_units("m/s");
   std::string speed = eval.execute();

The API also provides ``PUQConvert`` for converting an expression to target
units, ``PUQInfo`` for inspecting an expression, and ``PUQList`` for listing
units, quantities, and other PUQ definitions. Unit-system and physical-
quantity arguments can be supplied when the default inference is not enough.

.. code-block:: cpp

   #include "snt/api/puq_convert.h"

   snt::api::PUQConvert convert("1 mile", "km");
   std::string distance = convert.execute();

The command classes are useful when a C++ application needs the same
command-oriented behavior as the CLI. Applications that need typed results
can use the underlying :cpp:class:`snt::puq::Quantity` and related PUQ classes
directly. API errors are reported through the SNT exception hierarchy.

For the complete declaration and member reference, see the
:doc:`C++ PUQ API <cpp/api/puq>`.

