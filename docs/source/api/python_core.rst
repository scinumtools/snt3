Core Python API
===============

Core package definitions and shared settings.

Formatting settings are mutable Python attributes. Create a
``StringFormatType`` object and assign the fields you want to change:

.. code-block:: python

   from scinumtools3.core import StringFormatType

   format = StringFormatType()
   format.specifier = "f"
   format.valuePrecision = 3
   format.uncertaintyPrecision = 2
   format.paddingZeros = True

The configured object can then be passed to formatting functions and methods
that accept a format argument.

.. automodule:: scinumtools3
   :members:
   :undoc-members:

.. automodule:: scinumtools3.core
   :members:
   :undoc-members:

.. autoclass:: scinumtools3.core.StringFormatType
   :members:

Attributes
~~~~~~~~~~

``specifier``
   Numeric format specifier.
``valuePrecision``
   Number of digits displayed for the value.
``uncertaintyPrecision``
   Number of digits displayed for the uncertainty.
``thresholdScientific``
   Threshold at which scientific notation is selected.
``paddingZeros``
   Enables zero padding.
``paddingSize``
   Minimum padded field width.
``stringQuotes``
   Enables quotes around string values.

.. autoclass:: scinumtools3.core.DataType
   :members:
