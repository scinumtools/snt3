.. _api-application-interface:

API — Application Interface
---------------------------

The ``API`` module provides a standardized interface for exposing the
functionality of SciNumTools to external applications and systems. Its goal
is to provide a consistent way to work with PUEL and DIPL definitions
regardless of the environment in which they are used, separating the
scientific data model from the particular interface used to access it.

The API is intended to support different interfaces such as command-line
applications, console-based tools, REST services, and other integrations.
This allows the same PUEL quantities and DIPL parameter definitions to be
used consistently across different applications and services, providing a
common interface for defining, querying, evaluating, and exchanging
scientific data.

The ``snt::api`` C++ module provides command objects for these workflows.
They return formatted text suitable for interfaces such as the CLI. Code that
needs typed results can use ``snt::puq`` or ``snt::dip`` directly.

* :doc:`PUQ commands <puq>` — evaluate, convert, and inspect PUEL quantities and list definitions.
* :doc:`DIP commands <dip>` — parse DIPL, load and save DIPH5, and generate static parameters and reports.

.. toctree::
   :maxdepth: 1
   :hidden:

   puq
   dip
