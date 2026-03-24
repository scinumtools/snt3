# SciNumTools v3 Documentation

In this directory you can find the complete API reference and user guides for the ``SciNumTools3`` project.
The online documentation for the C++ reference implementation, including Python bindings and the CLI, is available [here](https://vrtulka23.github.io/scinumtools3/).

It also provides detailed specifications for the DIPL and PUEL domain-specific languages:

[DIPL](dipl/specification.md) is used to define validated, structured input parameters.  
[PUEL](puel/specification.md) defines a syntax for unit-aware expressions and calculations.  

## Building the documentation

To generate the documentation (Doxygen + Breathe + Sphinx), you can use either of the following methods:

* Using the setup script

  ```bash
  ./setup.sh -d   # builds the documentation
  ```
  
* Manually

  ```bash
  cd docs
  make html
  ```
