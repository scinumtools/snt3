Examples
========

The following examples demonstrate some of the core capabilities of
SciNumTools, from basic numerical values and physical quantities to
complete dimensional input parameter definitions.

Physical Quantities
-------------------

Create and manipulate physical quantities while keeping their units and
dimensions attached to the numerical value.

.. code-block:: python

   from scinumtools3.puq import Quantity
                
   length = Quantity(12.5, "m")
   width = Quantity(800, "mm")

   area = length + width
   print(area) # 13.3*m

Unit Systems and Conversion
---------------------------

SciNumTools supports multiple unit systems and conversion between compatible
units.

.. code-block:: python

   from scinumtools3.puq import Quantity

   distance = Quantity(10, "m")

   print(distance.convert("US_ft")) # 32.8084*ft

Uncertainties
-------------

Uncertainties can be associated with physical quantities and propagated
through calculations.

.. code-block:: python

   from scinumtools3.puq import Quantity
                
   length = Quantity("10.0(1)")
   width = Quantity("5.0(2)")

   area = length * width
   print(area)  # 5.00(25)e1

Expressions
-----------

Expressions can be parsed and evaluated using the EXS expression solver.
Specialized solvers can define their own operations and semantics.

.. code-block:: python

   from scinumtools3.puq import Calculator

   print(Calculator("2*au + 4*ly"))
   2.52966e5*au
   
Dimensional Input Parameters
----------------------------

DIP combines values, expressions, units, and constraints into a structured
parameter model.

.. code-block:: python

   from scinumtools3.dip import DIP

   dip = DIP()
   dip.add_string(
       "length float = 2.5 dm\n"
       "width float = 40 mm"
       "area float = ( {?length} * {?width} ) m2"
   )
   env = dip.parse()

   print(env['area'].value)  # 0.01

The same parameter definitions can then be evaluated and consumed by
different applications and interfaces.

