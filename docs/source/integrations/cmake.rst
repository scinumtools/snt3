CMake integration
=================

SciNumTools installs a CMake package named ``snt``. It provides the C++
libraries and the ``snt_dip_get`` helper from the CMake binding. The helper
evaluates one DIPL scalar during CMake configuration, so a DIPL file can
select CMake branches and target settings.

.. code-block:: cmake

   find_package(snt CONFIG REQUIRED)

   snt_dip_get(
       FILE config.dip
       PATH build.python
       TYPE BOOL
       OUTPUT ENABLE_PYTHON
   )

   if(ENABLE_PYTHON)
       find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
       add_subdirectory(python)
   endif()

The requested node must be one defined scalar without units or an array.
``TYPE`` may be ``BOOL``, ``INTEGER``, ``FLOAT``, or ``STRING`` and checks the
DIPL type without conversion. ``OUTPUT`` is a normal CMake variable; ``OUT``
is accepted as an alias. A DIPL boolean is returned as ``true`` or ``false``,
which works directly with CMake ``if()`` conditions.

Relative ``FILE`` paths are resolved from the current source directory. The
file is added to CMake's configure dependencies, so changing it causes CMake
to configure again. Additional DIPL files can be listed with ``DEPENDS``.

The installed command-line executable is selected automatically. Set
``SNT_EXECUTABLE`` to an absolute host executable when cross-compiling or when
a different SNT installation should be used. The executable is also exported
as the imported target ``snt::snt``.

The standalone example in ``examples/cmake-dipl`` demonstrates using a DIPL
boolean to create a target. The complete helper contract is documented in
``bindings/cmake/README.md``.
