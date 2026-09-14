C++ API interface
=================

The SNT API layer provides application-facing commands that use the same PUQ
and DIP implementations as the command-line tool. Direct C++ applications can
also link the ``snt-puq`` and ``snt-dip`` libraries and use their typed public
classes directly.

The API is built with the ``snt-api`` library and contains operations for
evaluating and converting PUQ expressions, querying PUQ information, listing
available definitions, and parsing DIPL input. A command is configured by
adding its inputs and requests, then executed once:

.. code-block:: cpp

   #include "snt/api/dip_parse.h"

   snt::api::DIPParse parse;
   parse.argument_add("file", {"config.dip"});
   parse.argument_request("build.python");
   parse.argument_value("bool");
   std::string value = parse.execute();

The API command classes return formatted text, while the underlying PUQ and
DIP C++ classes provide typed quantities, values, and environments for code
that needs to inspect or manipulate results directly. API errors are reported
through the SNT exception hierarchy and should be handled by the host
application. The command-oriented API headers are currently used from the SNT
source tree; installed applications normally use the public PUQ and DIP
headers or the Python binding.

For most applications, the Python binding or the CMake DIPL helper is more
convenient. Use the API layer when a C++ program needs the command-oriented
interface without launching the ``snt`` executable. The command-line tool,
Python binding, and API all evaluate the same PUEL and DIPL definitions.
