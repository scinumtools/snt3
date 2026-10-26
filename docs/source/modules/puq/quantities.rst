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

Distribution-aware uncertainty (opt in)
---------------------------------------

Ordinary quantity arithmetic uses fast first-order propagation. To examine a
nonlinear calculation under an assumed input distribution, call
``Quantity.monte_carlo`` explicitly. The callable receives sampled quantities
with the same units as the inputs. Reusing a sampled input preserves its
correlation, as in ``x * x``:

.. code-block:: python

   from scinumtools3.puq import Quantity

   x = Quantity(0.5, 0.4, "m")
   result = Quantity.monte_carlo(
       lambda x: x * x, [x], distributions=["normal"],
       samples=100_000, seed=42, coverage=0.95,
   )
   print(result.quantity)  # Sample mean and standard uncertainty, in m²
   print(result.lower, result.upper)  # Central 95% coverage interval

For several inputs, list the original quantities in the order expected by the
callable. They are sampled independently unless they share an uncertainty
source. Each sampled quantity retains its units, so ordinary PUQ arithmetic
still checks dimensions:

.. code-block:: python

   distance = Quantity(10.0, 0.2, "m")
   duration = Quantity(2.0, 0.1, "s")
   speed = Quantity.monte_carlo(
       lambda distance, duration: distance / duration,
       [distance, duration],
       distributions=["normal", "uniform"],
       samples=100_000, seed=42,
   )
   print(speed.quantity)             # Mean and standard uncertainty, in m/s
   print(speed.lower, speed.upper)  # Central 95% coverage interval, in m/s

To model correlated normal inputs, set their correlation *before* sampling:

.. code-block:: python

   first = Quantity(4.0, 0.2, "m")
   second = Quantity(3.0, 0.3, "m")
   Quantity.correlate(first, second, 0.8)
   difference = Quantity.monte_carlo(
       lambda a, b: a - b, [first, second], seed=42,
   )

The same source is drawn once per sample and reused wherever it appears. This
also applies when the same quantity is listed twice, for example
``Quantity.monte_carlo(lambda a, b: a - b, [x, x])`` has zero spread. Partially
correlated uniform inputs are not supported by this API.

``normal`` is the default distribution. Use ``"uniform"`` for a rectangular
input specified by its standard uncertainty: the sampled half-width is
``sqrt(3) * uncertainty``. The seed makes results reproducible. Inputs must be
scalar; the callable must produce one finite output value per sample. Domain
errors or non-finite samples raise an error rather than silently discarding
draws. Distribution-aware results depend on the assumed input distributions;
the default first-order calculations are unaffected.
