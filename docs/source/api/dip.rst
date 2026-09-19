DIP — C++ API interface
=======================

The DIP API provides the command-oriented C++ interface for parsing DIPL,
querying evaluated values, and loading or saving evaluated DIPH5 environments.
A command is configured by adding inputs and requests, then executed once.

Parsing and querying DIPL
-------------------------

.. code-block:: cpp

   #include "snt/api/dip_parse.h"

   snt::api::DIPParse parse;
   parse.argument_add("file", {"config.dip"});
   parse.argument_request("build.python");
   parse.argument_value("bool");
   std::string value = parse.execute();

Persisting an evaluated environment
------------------------------------

Use ``argument_save()`` to persist the full evaluated environment during
``execute()``. Use ``argument_load()`` instead of ``argument_add()`` to query
an existing DIPH5 file without reevaluating DIPL:

.. code-block:: cpp

   snt::api::DIPParse save;
   save.argument_add("string", {"simulation.steps int = 100"});
   save.argument_save("parameters.diph5");
   save.execute();

   snt::api::DIPParse load;
   load.argument_load("parameters.diph5");
   load.argument_request("simulation.steps");
   load.argument_value("integer");
   std::string steps = load.execute(); // "100\n"

Loading cannot be combined with DIPL file, string, source, or unit inputs.
Saving can accompany either parsing or loading and overwrites an existing
destination. Request and tag filters affect only text output, not the saved
environment. Output validation must succeed before saving. See
:doc:`DIP environment persistence <../modules/dip/persistence>` for the
format and its limitations.

Generating static parameters
----------------------------

Use ``argument_generate()`` to generate static parameters during
``execute()``. Its format is one of ``cpp``, ``c``, ``fortran``, ``rust``,
``julia``, ``json``, or ``yaml``:

.. code-block:: cpp

   snt::api::DIPParse generate;
   generate.argument_add("file", {"parameters.dip"});
   generate.argument_generate("cpp", "parameters.hpp");
   generate.execute();

Generation can follow parsing or DIPH5 loading, and can accompany saving.
It always exports the complete evaluated environment; request and tag filters
only affect text output. See :doc:`Static parameter generation
<../modules/dip/generation>` for the generated representations.

The API command classes return formatted text, while the underlying
``snt::dip`` classes provide typed environments and values for code that needs
to inspect or manipulate results directly. API errors are reported through
the SNT exception hierarchy.

For the complete declaration and member reference, see the
:doc:`C++ DIP API <cpp/api/dip>`.
