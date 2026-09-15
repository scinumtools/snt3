# SciNumTools v3 Documentation

In this directory you can find the complete API reference and user guides for the ``SciNumTools3`` project.
The online documentation for the C++ reference implementation, including Python bindings and the CLI, is available [here](https://vrtulka23.github.io/scinumtools3/).

It also provides detailed specifications for the DIPL and PUEL domain-specific languages:

[DIPL](dipl/specification.md) is used to define validated, structured input parameters.  
[PUEL](puel/specification.md) defines a syntax for unit-aware expressions and calculations.  

## Building the documentation

To generate the documentation (Doxygen + Breathe + Sphinx), you can use either of the following methods:

First install the package and documentation dependencies with
`python -m pip install ".[docs]"` from the repository root. The Python API
reference inspects the compiled bindings, so a source-only checkout is not
enough. A local build of the bindings using the same Python interpreter as
Sphinx is also supported.

Both commands below use `.venv/bin/python` when it exists, otherwise
`python3`. Sphinx must be installed in that environment. To select another
interpreter, use `PYTHON=/absolute/path/to/python ./setup.sh -d` or
`make -C docs html PYTHON=/absolute/path/to/python`.

* Using the setup script

  ```bash
  ./setup.sh -d   # builds the documentation
  ```
  
* Manually

  ```bash
  cd docs
  make html
  ```
