DIPfile projects
================

A ``DIPfile`` gathers the units, named sources, DIPL files, and inline DIPL
definitions used by one parameter environment. It is itself ordinary DIPL:
the project loader supplies the schemas for its three lists and parses the
file with the standard DIP parser.

Use a project from C++ with ``DIP::add_project()`` or from the command line:

.. code-block:: bash

   snt dip parse --project DIPfile --print

Project format
--------------

Each project contains only ``units[]``, ``sources[]``, and ``code[]`` items.
``code[]`` items retain their declared order. A code item has exactly one of
``file`` or ``string``.

.. code-block:: dipl

   units[]
     name = "length"
     unit = "23*au"

   sources[]
     name = "constants"
     filepath = "data/constants.dip"

   code[]
     file = "parameters/base.dip"

   code[]
     string = """
   derived int = ({?answer} + 1)
   """

Paths in ``filepath`` and ``file`` are interpreted relative to the directory
containing the DIPfile. The contents of a ``string`` block are passed to DIP
unchanged, so a root DIPL node inside the block must begin at column zero.

The project manifest is an input description rather than part of the final
parameter environment. It cannot be combined with individual ``--input``
arguments or a DIPH5 ``--load`` command.

Interfaces
----------

The same feature is available through all primary bindings:

.. code-block:: cpp

   snt::dip::DIP dip;
   dip.add_project("DIPfile");
   snt::dip::Environment env = dip.parse();

.. code-block:: python

   dip = DIP()
   dip.add_project("DIPfile")
   env = dip.parse()

In C, call ``snt_dip_parser_add_project(dip, "DIPfile", &error)`` before
``snt_dip_parser_parse``. The command-oriented C++ and Python API accepts
``argument_add("project", {"DIPfile"})`` and
``argument_add("project", ["DIPfile"])``, respectively.
