Python binding
==============

The Python binding exposes the SNT modules through the ``scinumtools3``
package. It is built on the same C++ implementation used by the command-line
and CMake interfaces, so PUQ and DIP expressions have the same semantics in
Python and C++.

After installation, import the module or one of its submodules:

.. code-block:: python

   import scinumtools3
   from scinumtools3.puq import Quantity
   from scinumtools3.dip import DIP

   length = Quantity(2.5, "m")
   print(length.convert("cm"))

Python values support scalar and array data, physical units, uncertainty
propagation, dimensional conversion, and DIPL parameter evaluation. The
package includes bindings for the core value layer, PUQ, DIP, and the public
API where those components are enabled in the build.

The binding follows the C++ module layout. The ``scinumtools3.val`` module
provides values and data containers, ``scinumtools3.exs`` evaluates
expressions, ``scinumtools3.puq`` handles quantities and units, and
``scinumtools3.dip`` handles dimensional input parameters. This keeps the
same concepts and names available when an application moves between the C++
and Python interfaces.

For application-oriented operations, use the Python API helpers exposed by
the installed package. They evaluate PUQ and DIPL definitions through the
same implementation as the ``snt`` command-line tool and the C++ API. The
Python binding is therefore the preferred interface when a Python program
needs typed results or direct access to module objects without starting a
subprocess.

For installation options and the complete Python API, see the
`Python binding README <https://github.com/vrtulka23/scinumtools3/tree/main/bindings/python>`_.
