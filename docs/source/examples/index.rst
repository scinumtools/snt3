Examples
========

The examples are grouped by module and build integration. Each page briefly
describes what the example demonstrates and how it is checked.

.. toctree::
   :maxdepth: 1

   exs
   puq
   dip

The module pages also provide short API snippets for the underlying concepts.

Applications using DIPL
-----------------------

`Nuclide Atlas <https://github.com/vrtulka23/nuclide-atlas>`_ is a unit-aware
nuclear-isotope database and decay-chain calculator. It uses DIPL for validated
nuclide records and scenarios, provenance, and CMake build configuration.

`H5Jive <https://github.com/vrtulka23/h5jive>`_ is a Rust desktop explorer for
HDF5 and NetCDF data. It uses SNT3's Rust bindings and DIPL for dataset source
catalogs, runtime settings, and compiled resource limits.

`Ad Imaginem <https://github.com/vrtulka23/adimaginem>`_ is a cross-domain
example: an interactive atlas of Thomistic philosophy and Catholic theology.
It uses DIPL to define ontology concepts, citations, and graph parameters.
