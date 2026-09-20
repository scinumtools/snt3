Quantities and arithmetic
=========================

PUQ represents a numerical value, optional uncertainty, and physical units as
one ``Quantity``. Units stay attached to the value while it is passed through
calculations, so incompatible additions and subtractions are rejected instead
of silently combining unrelated dimensions.

Constructing quantities
-----------------------

Construct a quantity from a value and a unit expression, or parse the complete
quantity from text. ``to_string()`` formats it for display:

.. code-block:: cpp

   #include <string>
   #include <snt/puq/quantity.h>

   snt::puq::Quantity length(2.5, "m");
   snt::puq::Quantity width("40 cm");
   std::string text = length.to_string();

For measured data, pass an absolute uncertainty in the same units as the
estimate:

.. code-block:: cpp

   snt::puq::Quantity mass(1.250, 0.005, "kg");

PUQ retains uncertainty and units in the resulting quantity. Values may also
be arrays; their shape is preserved by quantity operations and conversions.

Dimensional arithmetic
----------------------

The standard arithmetic operators combine dimensions according to unit
algebra. Multiplication and division derive new dimensions, while addition and
subtraction require compatible dimensions:

.. code-block:: cpp

   snt::puq::Quantity area = length * width;
   snt::puq::Quantity total_length = length + width;

The first result has area dimensions. The second is valid because metres and
centimetres are compatible; PUQ performs the necessary conversion. Adding a
mass to a length instead raises a PUQ error. The complete C++ declarations are
in the :doc:`PUQ C++ API <../../api/cpp/puq/quantities>`.
