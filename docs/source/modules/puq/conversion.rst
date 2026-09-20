Conversion and unit systems
===========================

Conversion creates a new ``Quantity`` expressed in target units. The original
quantity remains unchanged, which makes it safe to retain both a canonical
calculation value and a display-specific representation.

Converting compatible quantities
--------------------------------

Pass a target unit expression to ``Quantity::convert()``:

.. code-block:: cpp

   #include <snt/puq/quantity.h>

   snt::puq::Quantity distance("1 mile");
   snt::puq::Quantity distance_km = distance.convert("km");

The target must be dimensionally compatible with the source. PUQ reports an
error instead of converting, for example, a length to seconds. The same method
can convert derived units such as speed, energy, or pressure.

Systems and contextual conversions
----------------------------------

PUQ supports SI, US customary, ESU, and other registered unit systems. Most
conversions can infer all required information from the source and target
units. Some cross-system conversions need an explicit physical-quantity
context; the ``convert`` overload accepts a target system and quantity name
for that case.

The command-oriented C++ API supports selectable systems, formatted output,
and unit-definition lists. See :doc:`PUQ systems API
<../../api/cpp/puq/systems>` for the C++ system and conversion classes.
