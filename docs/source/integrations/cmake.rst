CMake integration
=================

SciNumTools installs a CMake package named ``snt``. It provides the C++
libraries and the ``snt_dip_get`` helper from the CMake binding.

Setup
-----

Load the package with ``find_package`` before using its targets or helpers.

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

Using DIPL values
-----------------

The requested node must be one defined scalar without units or an array.
``TYPE`` may be ``BOOL``, ``INTEGER``, ``FLOAT``, or ``STRING`` and checks the
DIPL type without conversion. ``OUTPUT`` is a normal CMake variable; ``OUT``
is accepted as an alias. A DIPL boolean is returned as ``true`` or ``false``,
which works directly with CMake ``if()`` conditions.

Relative ``FILE`` paths are resolved from the current source directory. The
file is added to CMake's configure dependencies, so changing it causes CMake
to configure again. Use ``DEPENDS`` when the selected value also depends on
other DIPL files, such as imported sources or a shared configuration file.
Each file listed there is registered with CMake's configure dependency graph;
when one changes, CMake reruns configuration and reevaluates the DIPL value.

Advanced usage
--------------

The installed command-line executable is selected automatically. Set
``SNT_EXECUTABLE`` to an absolute host executable when cross-compiling or when
a different SNT installation should be used. The executable is also exported
as the imported target ``snt::snt``.

The standalone example in ``examples/dip/CMakeIntegration`` demonstrates using a DIPL
boolean to create a target. The complete helper contract is documented in
``bindings/cmake/README.md``.

Different scalar types can be read without conversion by declaring the
expected ``TYPE``. Additional files may be tracked with ``DEPENDS``:

.. code-block:: cmake

   snt_dip_get(FILE config.dip PATH project.name TYPE STRING OUT PROJECT_NAME)
   snt_dip_get(
       FILE config.dip
       PATH project.jobs
       TYPE INTEGER
       OUTPUT PROJECT_JOBS
       DEPENDS toolchain.dip
   )

For lower-level use, invoke the installed executable directly. The
``SNT_EXECUTABLE`` variable can override the automatically selected tool:

.. code-block:: cmake

   execute_process(
       COMMAND "${SNT_EXECUTABLE}" dip parse
               --input config.dip
               --request "?project.name"
               --value
       OUTPUT_VARIABLE PROJECT_NAME
       OUTPUT_STRIP_TRAILING_WHITESPACE
       COMMAND_ERROR_IS_FATAL ANY
   )
