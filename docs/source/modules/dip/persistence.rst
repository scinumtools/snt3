Environment Persistence
=======================

DIP can persist an evaluated environment in the DIPH5 HDF5 format. This is
useful when a DIPL input has already been parsed and evaluated and the
resulting values need to be exchanged with another application, stored for a
later calculation, or inspected by legacy scientific software using HDF5.

The persisted object is an evaluated environment, not the original DIPL
program or the complete parser runtime. The format stores values, hierarchy,
collections, units, node settings, and provenance; current round-trip
limitations are listed below. Runtime function definitions,
branching state, and complete parser registries are not reconstructed.

The recommended filename extension is ``.diph5``. The extension is only a
naming convention; the file is identified normatively by its root
``_DIPL_Format`` and ``_DIPL_Schema_Version`` attributes.

Saving and loading
------------------

The C++ API saves and loads through :cpp:class:`snt::dip::Environment`:

.. code-block:: cpp

   snt::dip::DIP parser;
   parser.add_file("parameters.dipl");
   snt::dip::Environment environment = parser.parse();

   environment.save("parameters.diph5");

   snt::dip::Environment restored;
   restored.load("parameters.diph5");

For usage through language bindings, see
:doc:`Interfaces and integrations <../../integrations/index>`.

``save()`` overwrites an existing destination file. ``load()`` replaces the
current environment only after the file has been read successfully; a failed
load leaves the existing environment unchanged.

Current limitations
-------------------

The current implementation does not yet preserve every detail on a save/load
round trip:

* Single-element arrays lose their array classification when loaded.
* Units associated with null values are not restored by the loader.
* Groups and collections without value-node descendants are not written:
  the writer constructs the hierarchy from the saved value paths.

Source identifiers, line numbers, and captured source text remain node-level
provenance. Loading does not reopen the original source files or restore the
source registry, so full source-text retrieval and source-qualified lookups
are not available from that provenance alone.

HDF5 mapping
------------

Fully qualified DIPL paths become HDF5 paths. Hierarchy and collection nodes
are groups, while value nodes are datasets:

.. code-block:: text

   /simulation                         group   (_DIPL_Kind=group)
   /simulation/steps                   dataset (_DIPL_Kind=value)
   /boundary                           group   (_DIPL_Kind=map)
   /boundary/inlet                     group   (_DIPL_Kind=map_item)
   /boundary/inlet/velocity            dataset (_DIPL_Kind=value)
   /samples                            group   (_DIPL_Kind=list)
   /samples/0                          group   (_DIPL_Kind=list_item)
   /samples/0/time                     dataset (_DIPL_Kind=value)

The ``_DIPL_*`` attributes carry the DIPL meaning that cannot be inferred
from HDF5 names alone. Value datasets use native HDF5 numeric datatypes,
simple dataspaces for arrays, and null dataspaces for null values. Node
settings and provenance are stored as dataset attributes.

* `Source specification <https://github.com/vrtulka23/scinumtools3/blob/main/docs/diph5/specification.md>`_

Full DIPH5 specification
------------------------

The complete format contract, including object kinds, value datatypes,
attributes, compatibility rules, and DIPL-to-HDF5 examples, is provided in
the standalone specification below.

.. raw:: html

   <iframe
       src="../../_static/diph5-specification.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
   </br></br>
