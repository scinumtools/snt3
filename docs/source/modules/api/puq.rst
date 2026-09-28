.. _module-api-puq:

PUQ commands
============

``PUQEval`` evaluates a PUEL expression. Configure the requested output
before calling ``execute()``:

.. code-block:: cpp

   #include <snt/api/puq_eval.h>

   snt::api::PUQEval eval("12*km / 3*h");
   eval.argument_output_units("m/s");
   std::string speed = eval.execute();

``PUQConvert`` converts an expression to target units. ``PUQInfo`` inspects
an expression, and ``PUQList`` lists units, quantities, and other
definitions. Unit-system and physical-quantity arguments can be supplied
when default inference is not enough:

.. code-block:: cpp

   #include <snt/api/puq_convert.h>

   snt::api::PUQConvert convert("1 mile", "km");
   std::string distance = convert.execute();

These commands use the same PUEL and PUQ implementations as the CLI. Use
:cpp:class:`snt::puq::Quantity` directly when a C++ application needs a typed
quantity. See the :doc:`generated PUQ command declarations
<../../api/cpp/api/puq>` for all members.
