Installation
============

SciNumTools v3 can be installed in several ways, depending on how it is
intended to be used. Command-line users can download a precompiled ``snt``
executable from GitHub Releases. Python users can install the Python bindings
from PyPI or Conda, while C++ users can use vcpkg, Conan, Homebrew, or build
the library directly from source.

For most users, installing a pre-built package is recommended. Building
from source is useful when developing SciNumTools itself, requiring specific
build options, or integrating a development version of the library.


Precompiled ``snt`` executable
------------------------------

Download the archive for your system from the `latest GitHub release
<https://github.com/scinumtools/snt3/releases/latest>`_, or use the installer
for Linux x86-64 and macOS ARM64. It finds the latest release and installs
``snt`` under ``~/.local`` without administrator rights:

.. code-block:: console

   curl -fsSLO https://raw.githubusercontent.com/scinumtools/snt3/main/packaging/installers/install-cli.sh
   sh install-cli.sh

The Linux executable requires system runtime libraries, including libcurl,
libsz.so.2 (Ubuntu package ``libsz2``), and libGL.so.1 (Ubuntu package
``libgl1``). The installer lists missing Linux libraries before activating the
executable, and checks that it runs. If ``~/.local/bin`` is not already on
``PATH``, add it to your shell configuration:

.. code-block:: console

   export PATH="$HOME/.local/bin:$PATH"

Use ``sh install-cli.sh --version vX.Y.Z`` to install a specific release, or
``sh install-cli.sh --prefix /path/to/prefix`` for a shared project directory.
On a cluster without network access, download the archive on a login node and
run ``sh install-cli.sh --archive /path/to/snt-vX.Y.Z-linux-x86_64.tar.gz``.

To remove all releases installed by this script, run:

.. code-block:: console

   sh install-cli.sh --uninstall

Add ``--version vX.Y.Z`` to remove only one release, and use the same
``--prefix`` value used for installation when installing outside ``~/.local``.
Removing the active release also removes its ``snt`` command symlink. Other
files in the prefix are preserved.

On Windows x86-64, use the PowerShell installer. It installs into the current
user's profile and adds its command directory to the user ``PATH``:

.. code-block:: powershell

   Invoke-WebRequest https://raw.githubusercontent.com/scinumtools/snt3/main/packaging/installers/install-cli.ps1 -OutFile install-cli.ps1 -UseBasicParsing
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\install-cli.ps1

Use ``-Version vX.Y.Z``, ``-Prefix C:\path\to\prefix``, or ``-Archive
C:\path\to\snt-vX.Y.Z-windows-x86_64.zip`` for the corresponding options.
Open a new terminal after installation to use ``snt`` by name.

To remove all releases installed by the PowerShell script, run:

.. code-block:: powershell

   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\install-cli.ps1 -Uninstall

Add ``-Version vX.Y.Z`` to remove only one release, and pass the original
``-Prefix`` if you used a custom location. The installer removes its managed
command shim and the user ``PATH`` entry it added when no installed command
remains. Open a new terminal to see the updated ``PATH``.

The archives contain the command-line application, not the C++ development
libraries or Python bindings. The ``snt hub`` command additionally requires
Git and Python 3 to fetch and run project adapters.
The release archives include the ``snt view`` desktop viewer. It requires a
working graphical desktop and OpenGL support on the target machine.


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

To put ``snt`` on ``PATH`` in a consuming project, add the versioned
``scinumtools3`` package created above under ``[tool_requires]`` in its
``conanfile.txt`` and generate ``VirtualBuildEnv``. A regular library
requirement still provides the executable in the package's ``bin`` directory,
but does not add that directory to the runtime environment.


macOS / Homebrew
~~~~~~~~~~~~~~~~

On macOS, SciNumTools can be installed using the project Homebrew tap:

.. code-block:: console

   brew tap scinumtools/tap
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
* **Command-line application:** download the precompiled ``snt`` executable
  from GitHub Releases.
* **C++ application:** use vcpkg, Conan, Homebrew, or a system installation.
* **SciNumTools development:** build directly from source.
* **Reproducible development environment:** use the :doc:`Docker integration
  <integrations/docker>`.

After installation, continue with :doc:`quickstart` to start using
SciNumTools.
