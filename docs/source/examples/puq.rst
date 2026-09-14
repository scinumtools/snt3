PUQ examples
============

CBinding
--------

The ``examples/puq/CBinding`` directory is a standalone C project using only
the public ``snt/c.h`` interface for PUQ operations. It evaluates a quantity,
converts it to centimetres, formats the result into a caller-provided buffer,
and releases the opaque handles through the C ABI.

See the `CBinding source directory <https://github.com/vrtulka23/scinumtools3/tree/main/examples/puq/CBinding>`_.

.. code-block:: c

   snt_quantity* quantity = NULL;
   snt_quantity* converted = NULL;
   snt_quantity_eval("2.5 m", &quantity, &error);
   snt_quantity_convert(quantity, "cm", &converted, &error);
   snt_quantity_format(converted, output, sizeof(output), &error);
