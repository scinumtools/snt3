Installation
============

SciNumTools v3 can be installed in several ways, depending on how it is
intended to be used. Python users can install the Python bindings directly
from PyPI or Conda, while C++ users can use vcpkg, Conan, Homebrew, or build
the library directly from source.

For most users, installing a pre-built package is recommended. Building
from source is useful when developing SciNumTools itself, requiring specific
build options, or integrating a development version of the library.


Python
------

The Python bindings are available as a regular Python package on PyPI:

.. code-block:: console

   pip install scinumtools3

SciNumTools is also available through conda-forge:

.. code-block:: console

   conda install conda-forge::scinumtools3

After installation, the Python interface can be imported directly:

.. code-block:: python

   from scinumtools3.puq import Quantity
   from scinumtools3.dip import DIP


C++
---

For C++ projects, SciNumTools can be installed using several package
managers, including vcpkg and Conan.


vcpkg
~~~~~

SciNumTools is available as a vcpkg package:

.. code-block:: console

   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg

   ./bootstrap-vcpkg.sh

   ./vcpkg install scinumtools3

The local vcpkg port builds report support with the ``reports`` feature:
``./vcpkg install 'scinumtools3[reports]'``. The feature fetches Brief++.

On Windows, use ``bootstrap-vcpkg.bat`` instead of
``bootstrap-vcpkg.sh``.


Conan
~~~~~

SciNumTools can also be created as a local Conan package directly from the
source repository:

.. code-block:: console

   git clone --recurse-submodules https://github.com/scinumtools/snt3.git
   cd snt3

   conan create .

The Conan package installs the C++ libraries and the ``snt`` executable with
server, viewer, and report support enabled by default. The viewer requires the
platform's OpenGL and window-system development libraries. Conan options
``with_server=False``, ``with_viewer=False``, and ``with_reports=False`` disable
the corresponding feature and its bundled dependency. For example, create a
package without the three features using:

.. code-block:: console

   conan create . -o 'scinumtools3/*:with_server=False' -o 'scinumtools3/*:with_viewer=False' -o 'scinumtools3/*:with_reports=False'

To put ``snt`` on ``PATH`` in a consuming project, add
``scinumtools3/0.9.0`` under ``[tool_requires]`` in its ``conanfile.txt`` and
generate ``VirtualBuildEnv``. A regular library requirement still provides
the executable in the package's ``bin`` directory, but does not add that
directory to the runtime environment.


macOS / Homebrew
~~~~~~~~~~~~~~~~

On macOS, SciNumTools can be installed using the project Homebrew tap:

.. code-block:: console

   brew tap vrtulka23/tap
   brew install scinumtools3


Building from Source
--------------------

SciNumTools is built using CMake and requires a C++17-compatible compiler.
Building from source provides full access to the C++ library, command-line
applications, tests, and other components of the project.
For a summary of required and included build dependencies, see
:doc:`dependencies`.

With DIP enabled (the default), install HDF5 development headers and the
HDF5 C library before configuring. The HDF5 C++ library is not required.
If HDF5 is installed outside the standard search paths, pass its installation
prefix as ``-DHDF5_ROOT=/path/to/hdf5`` when running CMake. The commands below
also require the Ninja build tool.

Clone the repository with its submodules and configure the build. The
``briefpp`` submodule provides the header-only renderer for DIP reports.
Alternatively, set ``SNT_BRIEFPP_INCLUDE_DIR`` to a directory containing
``briefpp/report.hpp``. The ``cpp-httplib`` submodule provides the optional REST
server. Alternatively, install cpp-httplib 0.46.0 or newer and set
``SNT_HTTPLIB_INCLUDE_DIR`` to its header directory. Builds with
``-DENABLE_SNT_SERVER=OFF`` do not need cpp-httplib.
Builds with ``-DENABLE_SNT_REPORT=OFF`` omit report generation and do not need
Brief++. The report option defaults to on when DIP is enabled.

.. code-block:: console

   git clone --recurse-submodules https://github.com/scinumtools/snt3.git
   cd snt3

   cmake -G Ninja -B build
   cmake --build build

The test suite can then be executed using CTest:

.. code-block:: console

   ctest --test-dir build

Finally, install the compiled library and associated components:

.. code-block:: console

   cmake --install build


Using the Setup Script
~~~~~~~~~~~~~~~~~~~~~~

The repository also provides a convenience setup script for building,
testing, and installing SciNumTools:

.. code-block:: console

   sudo ./setup.sh -b -t -i

Here ``-b`` builds the project, ``-t`` runs the tests, and ``-i`` installs
the resulting components.


Using SciNumTools from CMake
----------------------------

Once installed, SciNumTools can be discovered from another CMake project
using ``find_package``:

.. code-block:: cmake

   find_package(snt REQUIRED)

Individual SciNumTools modules can then be linked to an executable:

.. code-block:: cmake

   add_executable(${EXEC_NAME} ${SOURCE_FILES})

   target_link_libraries(
       ${EXEC_NAME}
       PRIVATE
       snt-exs
       snt-puq
       snt-dip
   )

This modular approach allows an application to link only the SciNumTools
components it requires.


Choosing an Installation Method
-------------------------------

The recommended installation method depends on the intended use:

* **Python application:** install ``scinumtools3`` from PyPI or Conda.
* **C++ application:** use vcpkg, Conan, Homebrew, or a system installation.
* **SciNumTools development:** build directly from source.
* **Reproducible development environment:** use the :doc:`Docker integration
  <integrations/docker>`.

After installation, continue with :doc:`quickstart` to start using
SciNumTools.
