VAL and Python values
=====================

VAL is the internal SNT value layer. It provides the C++ data
representation used by modules such as PUQ and DIP, but it is not intended
to be a public Python API.

Python applications should use ordinary Python values for scalar data and
lists or NumPy arrays for array data. The PUQ and DIP bindings convert these
values at their boundaries and return normal Python-compatible results.

The internal ``scinumtools3.val`` submodule may exist in a build, but its
implementation types are not documented as part of the supported Python
interface.
