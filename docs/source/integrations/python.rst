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

For installation options and the complete Python API, see the
`Python binding README <https://github.com/vrtulka23/scinumtools3/tree/main/bindings/python>`_.
