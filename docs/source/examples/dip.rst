DIP examples
============

The DIP examples cover both a minimal DIPL definition and integration of DIPL
configuration with a CMake build.

QuickExample
------------

The ``examples/dip/QuickExample`` directory contains a small DIPL definition
with typed parameters, units, references, and validation properties. It is a
smoke test: the C++ and Python quick-example tests load the README example and
verify that it parses and evaluates successfully.

See the `QuickExample source directory <https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/QuickExample>`_.
The test does not exercise a separate application workflow; it checks that
this compact definition remains valid and produces an evaluable parameter
tree.

.. code-block:: dipl

   length float = 2.5 m
   width float = 40 cm
   area float = ({?length} * {?width})
     !condition ({.} > 0 m2)

CMakeIntegration
----------------

The ``examples/dip/CMakeIntegration`` directory is a standalone CMake
consumer. It reads boolean, integer, and string values from ``build.dip``
with ``snt_dip_get`` and uses them to configure a C++ target and its standard.

See the :doc:`../integrations/cmake` guide for the commands and configuration
details. The complete source is available in the
`CMakeIntegration directory <https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/CMakeIntegration>`_.

The example shows configure-time evaluation with ``snt_dip_get``. A boolean
controls whether the executable target is created, an integer selects its C++
standard, and a string is printed during configuration. Editing ``build.dip``
causes CMake to reconfigure and apply the new settings.

.. code-block:: cmake

   snt_dip_get(FILE build.dip PATH build.tools TYPE BOOL OUTPUT BUILD_TOOLS)
   snt_dip_get(FILE build.dip PATH build.standard TYPE INTEGER OUTPUT CXX_STANDARD)
   if(BUILD_TOOLS)
       add_executable(dip-cmake-example main.cpp)
       set_property(TARGET dip-cmake-example PROPERTY CXX_STANDARD "${CXX_STANDARD}")
   endif()

C binding
---------

The `DIP C binding example <https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/CBinding>`_
uses only ``snt/c.h`` to add DIPL text, parse it, and retrieve the ``answer``
node through an opaque C handle.

.. code-block:: c

   snt_dip_create(&dip, &error);
   snt_dip_add_string(dip, "answer int = 42", &error);
   snt_dip_parse(dip, &error);
   snt_dip_get(dip, "answer", value, sizeof(value), &error);
