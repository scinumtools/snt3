Dependencies
============

What you need depends on how you use SciNumTools. See :doc:`installation` for
installation and build commands.

Installing a package
--------------------

For Python, ``pip install scinumtools3`` installs the package and its declared
Python dependency, NumPy. Conda and the C++ package managers described in the
installation guide resolve their packaged dependencies.

Building from source
--------------------

* **Build tools:** a C++17 compiler and CMake 3.22 or newer. Install Ninja if
  using the ``cmake -G Ninja`` commands in the installation guide.
* **DIP:** HDF5 development headers and the HDF5 C library are required when
  DIP is enabled, which is the default. The HDF5 C++ library is not required.
* **Python bindings:** the default build enables them and needs a Python 3
  interpreter and development files. CMake uses an installed pybind11 or
  downloads it when needed. Set ``-DENABLE_BINDING_PYTHON=OFF`` for a build
  without Python bindings.
* **Tests:** the default build enables tests. CMake uses an installed
  GoogleTest or downloads it; the Python tests also need pytest 8.2.1 or
  newer. Set ``-DENABLE_UNIT_TESTS=OFF`` if tests are not needed.
* **Viewer (optional):** ``-DENABLE_SNT_VIEW=ON`` needs OpenGL development
  files and the GLFW and Dear ImGui submodules. Linux also needs
  ``pkg-config`` to locate OpenGL. GLFW builds its Wayland backend by
  default, which needs Wayland development files,
  ``libxkbcommon``, and ``wayland-scanner`` (provided by ``libwayland-bin``
  on Debian and Ubuntu). By default, the X11 backend is built when its
  development library is found; otherwise the build continues with Wayland. See
  :doc:`integrations/viewer` for the Debian and Ubuntu package command.

Included with the source checkout
---------------------------------

Clone with ``--recurse-submodules`` to include Brief++ for DIP reports,
cpp-httplib for ``snt server``, and GLFW and Dear ImGui for ``snt view``.
Brief++ is needed when DIP is enabled; the other submodules are needed only
when their corresponding optional commands are enabled. An ordinary clone
can fetch them later with ``git submodule update --init --recursive``. The
installation guide also describes how to use separately installed copies
of Brief++ and cpp-httplib.

Optional tools
--------------

PDF report generation needs a local TeX compiler such as ``pdflatex``.
Building this documentation needs Doxygen, Sphinx, Breathe, the Sphinx theme,
and a compiled ``scinumtools3`` Python binding. The Python documentation
dependencies are listed in ``.[docs]``. See :doc:`modules/dip/report` for
report requirements.
