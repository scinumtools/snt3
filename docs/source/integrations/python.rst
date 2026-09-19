Python binding
==============

The Python binding exposes the SNT modules through the ``scinumtools3``
package. It is built on the same C++ implementation used by the command-line
and CMake interfaces, so PUQ and DIP expressions have the same semantics in
Python and C++.

After installation, import the module or one of its submodules:

PUQ quantities
--------------

PUQ represents numerical values together with their physical units and,
when needed, uncertainties. Quantities can be added, multiplied, compared,
formatted, and converted between compatible unit systems while retaining
dimensional information. Use it when calculations should prevent accidental
mixing of incompatible units.

.. code-block:: python

   from scinumtools3.puq import Quantity
   from scinumtools3.dip import DIP

   length = Quantity(2.5, "m")
   print(length.convert("cm"))

DIP parameters
--------------

The Python binding can load a DIPL definition from a file, evaluate its
parameters, and return a value in Python. For example, given a file named
``parameters.dip``:

.. code-block:: dipl

   length float = 2.5 dm
   width float = 40 mm
   area float = ( {?length} * {?width} ) m2

Parse and query it with ``DIP.add_file`` and the environment cursor:

.. code-block:: python

   from pathlib import Path
   from scinumtools3.dip import DIP

   dip = DIP()
   dip.add_file(Path("parameters.dip"))
   env = dip.parse()

   area = env["area"]
   print(area.value, area.units)  # 0.01 m2

The cursor also provides values and units for unitless scalars. For example,
a file containing ``count int = 42`` can be queried with
``env["count"].value``. The cursor returned by ``env["area"]`` exposes both
``value`` and ``units``; pass the value to ``Quantity`` when an explicit unit
conversion is needed:

.. code-block:: python

   from scinumtools3.puq import Quantity

   area_in_cm2 = Quantity(area.value, area.units).convert("cm2")
   print(area_in_cm2)  # 100 cm2

Persisting DIP environments
----------------------------

Save evaluated parameters to a DIPH5 file and restore them through
``Environment``:

.. code-block:: python

   from scinumtools3.dip import DIP, Environment

   parser = DIP()
   parser.add_string("simulation.steps int = 100")
   env = parser.parse()
   env.save("parameters.diph5")

   restored = Environment()
   restored.load("parameters.diph5")
   assert restored["simulation.steps"].value == 100

See :doc:`Environment persistence <../modules/dip/persistence>` for the
format, save/load behavior, and current limitations.

The command-oriented API also supports persistence. Configure the command
with ``argument_save()`` or ``argument_load()``; the file is written or read
when ``execute()`` is called:

.. code-block:: python

   from scinumtools3.api.dip import DIPParse

   save = DIPParse()
   save.argument_add("string", ["simulation.steps int = 100"])
   save.argument_save("parameters.diph5")
   save.execute()

   load = DIPParse()
   load.argument_load("parameters.diph5")
   load.argument_request("simulation.steps")
   load.argument_value("integer")
   assert load.execute() == "100\n"

``argument_load()`` cannot be combined with ``argument_add()``. Saving
overwrites the destination and always includes the full environment,
regardless of request or tag filters. Output validation must succeed before
saving. These command methods take file paths as strings; use ``str(path)``
for a ``pathlib.Path``.

Generating static parameters
----------------------------

Export an evaluated environment with ``Environment.generate()`` when a
validated DIPL configuration should become a source or data file consumed by
another application. Import ``ExportFormat`` from ``scinumtools3.dip`` and
choose one of ``CPP``, ``C``, ``FORTRAN``, ``RUST``, ``JULIA``, ``JSON``, or
``YAML``:

.. code-block:: python

   from scinumtools3.dip import DIP, ExportFormat

   parser = DIP()
   parser.add_string("simulation.steps int = 100")
   env = parser.parse()
   env.generate(ExportFormat.CPP, "parameters.hpp")
   env.generate(ExportFormat.JSON, "parameters.json")

The command-oriented Python API can generate during ``execute()`` as well:

.. code-block:: python

   from scinumtools3.api.dip import DIPParse

   generate = DIPParse()
   generate.argument_add("file", ["parameters.dip"])
   generate.argument_generate("rust", "parameters.rs")
   generate.execute()

Generation always exports the complete evaluated environment; requests and
tags only restrict textual output. See :doc:`Static parameter generation
<../modules/dip/generation>` for native representations and format-specific
behavior.

Python values and NumPy
-----------------------

The C++ VAL layer maps to ordinary Python values: ``int``, ``float``,
``str``, ``bool``, lists, and ``numpy.ndarray``. There is no separate VAL
value class to learn, so results work directly with normal Python and NumPy
code.

Why EXS is not exposed
----------------------

EXS is used internally by PUQ and DIP, but is not exposed as a standalone
Python module. Python already has mature tools for custom expression
evaluation and grammars; a direct EXS binding would be useful mainly when the
same custom language must run across Python, C++, the CLI, and REST services.

For application-oriented operations, use the Python API helpers exposed by
the installed package. They evaluate PUQ and DIPL definitions through the
same implementation as the ``snt`` command-line tool and the C++ API. The
Python binding is therefore the preferred interface when a Python program
needs typed results or direct access to module objects without starting a
subprocess.

For installation options and the complete Python API, see the
`Python binding README <https://github.com/vrtulka23/scinumtools3/tree/main/bindings/python>`_.
