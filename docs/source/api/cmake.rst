CMake API
=========

The installed package provides CMake integration for projects that use DIPL
files to control build configuration.

Package
-------

.. code-block:: cmake

   find_package(scinumtools3 REQUIRED)

The package exports the ``snt::snt`` executable target and the
``SNT_EXECUTABLE`` path variable.

Helper function
---------------

``snt_dip_get`` reads one scalar value during CMake configuration:

.. code-block:: cmake

   include(SNTDIPL)
   snt_dip_get(
       FILE ${CMAKE_CURRENT_SOURCE_DIR}/config.dip
       PATH build.python
       TYPE BOOL
       OUTPUT ENABLE_PYTHON
   )

Boolean results are emitted as ``true`` or ``false``:

.. code-block:: cmake

   if(ENABLE_PYTHON)
       find_package(Python3 REQUIRED COMPONENTS Interpreter)
   endif()

``snt_dip_get`` accepts the following arguments:

Arguments: ``FILE`` is the DIPL file, ``PATH`` is the node path, ``OUTPUT``
(``OUT``) is the result variable, ``TYPE`` is ``BOOL``, ``INTEGER``,
``FLOAT``, or ``STRING``, and ``DEPENDS`` lists additional configure
dependencies.

Errors are reported during configuration when the file, node, or scalar
value cannot be evaluated. Relative ``FILE`` paths use
``CMAKE_CURRENT_SOURCE_DIR``.
