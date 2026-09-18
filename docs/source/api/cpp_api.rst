Application Interface C++ API
=============================

The ``snt::api`` module supplies command-oriented operations for building
interfaces such as the CLI and REST services. It includes PUQ evaluation,
conversion, information and listing commands, DIPL parsing, and exceptions.

Command headers currently live in ``src/snt/api``; the exception header is
in ``include/snt/api``. See :doc:`puq` and :doc:`dip` for usage details.

.. toctree::
   :maxdepth: 1

   cpp/api/puq
   cpp/api/dip
   cpp/api/errors
