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

On Windows, use ``bootstrap-vcpkg.bat`` instead of
``bootstrap-vcpkg.sh``.


Conan
~~~~~

SciNumTools can also be created as a local Conan package directly from the
source repository:

.. code-block:: console

   git clone https://github.com/vrtulka23/scinumtools3.git
   cd scinumtools3

   conan create .


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

Clone the repository and configure the build:

.. code-block:: console

   git clone https://github.com/vrtulka23/scinumtools3.git
   cd scinumtools3

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


Docker
------

Preconfigured Docker images are provided for both Python users and
SciNumTools development.

The Python image can be built with:

.. code-block:: console

   docker build \
       -f packaging/docker/python/Dockerfile \
       -t scinumtools3-python .

For development, use:

.. code-block:: console

   docker build \
       -f packaging/docker/dev/Dockerfile \
       -t scinumtools3-dev .

The Docker setup is particularly useful when a reproducible development
environment is required or when installing all build dependencies locally
would be inconvenient.


Choosing an Installation Method
-------------------------------

The recommended installation method depends on the intended use:

* **Python application:** install ``scinumtools3`` from PyPI or Conda.
* **C++ application:** use vcpkg, Conan, Homebrew, or a system installation.
* **SciNumTools development:** build directly from source.
* **Reproducible development environment:** use the provided Docker images.

After installation, continue with :doc:`quickstart` to start using
SciNumTools.
