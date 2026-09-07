Quick Start
===========

This quick start introduces the basic workflow of SciNumTools through a few
small examples. It starts with physical quantities and expressions and then
moves to dimensional input parameters using DIPL.

The examples below use the Python interface. The same concepts are available
through the C++ API and other SciNumTools interfaces.


Working with Physical Quantities
--------------------------------

A physical quantity combines a numerical value with its unit. SciNumTools
keeps the physical meaning of the value throughout calculations.

.. code-block:: python

   from scinumtools3.puq import Quantity

   length = Quantity(2.5,"m")
   width = Quantity(40,"cm")

   area = length * width

   print(area)   # 100*m*cm

The result retains its physical dimension and can be converted to another
compatible unit:

.. code-block:: python

   print(area.convert("cm2"))  # 1e4*cm2

Expressions and Units
---------------------

PUEL expressions can also be evaluated directly. This makes it possible to
work with quantities using a compact textual representation:

.. code-block:: python

   from scinumtools3.puq import Calculator

   print(Calculator("2.5*m + 40*cm"))  # 2.9*m

The Calculator in PUQ and expression solver in DIP perform the required unit conversion and dimensional
operations automatically.

Defining Input Parameters
-------------------------

DIP extends the concept of physical quantities to complete scientific input
models. A parameter can contain a value, type, unit, default value,
constraints, and relationships to other parameters.

For example, a simple parameter definition can describe the dimensions of a
rectangular object:

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

The resulting parameter environment evaluates the dependency between
``length``, ``width``, and ``area``. The unit of area is explicitly defined
by its declaration, while the values of length and width are
automatically converted as needed during the calculation.

The parameter model can also be defined independently of the application
using a structured definition. This provides an explicit description of the
available parameters, their properties, and their relationships.

This separates the parameter model from the application that consumes it.
The same definition can therefore be evaluated from C++, Python, the
snt command-line interface, or through the API interfaces.

Where to Go Next
----------------

This quick start only demonstrates the basic concepts. The following
sections introduce each part of SciNumTools in more detail:

* :doc:`puel/index` — learn the PUEL language and physical quantity
  expressions.
* :doc:`dipl/index` — define structured dimensional input parameters with
  DIPL.
* :doc:`modules/index` — understand the architecture and individual modules.
* :doc:`examples/index` — explore more complete examples and use cases.
* :doc:`api/index` — access the detailed C++ and Python API reference.
