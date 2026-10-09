Environment Persistence
=======================

DIP can persist an evaluated environment in the DIPH5 HDF5 format. This is
useful when a DIPL input has already been parsed and evaluated and the
resulting values need to be exchanged with another application, stored for a
later calculation, or inspected by scientific software using HDF5.

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
   snt::dip::Environment env = parser.parse();

   env.save("parameters.diph5");

   snt::dip::Environment restored;
   restored.load("parameters.diph5");

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

Source identifiers, line numbers, captured source lines, and citation metadata
remain node-level provenance. DIPH5 version 2 stores a source manifest with
each source's name, recorded path, parent relationship, and SHA-256 hash of
the exact parsed content. Source paths can be saved relative to the DIPH5
directory with ``SnapshotSaveOptions`` and
``SourcePathPolicy::RelativeToSnapshot``. Relative paths in a loaded snapshot
are interpreted from that snapshot's directory when it is saved again with
the same policy. The default preserves the original path strings.
Version 2.1 additionally preserves the trace IDs of registered units, schemas,
and functions. Version 2.4 adds schema-level
descriptions, citations, and source locations to those trace entries. In C++,
``get_schema_manifest()`` returns these descriptive records and
``get_applied_schemas(path)`` finds schemas applied along a value path.
``get_contributing_schema(path)`` identifies the schema that supplied a value
node, when known. Loading
does not recreate complete source text, parsed source nodes, schema definitions,
executable functions, or source-qualified lookups; a loaded schema cannot be
instantiated from the snapshot.

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

* `Source specification <https://github.com/scinumtools/snt3/blob/main/docs/diph5/specification.md>`_

.. _dip-diph5-comparison:

Comparing DIPH5 snapshots
-------------------------

Construct a :cpp:class:`snt::dip::Comparison <snt::dip::snt::dip::Comparison>` with two DIPH5 snapshots. It
loads and compares them once, then lets you inspect or render the result as
often as needed. The result contains added, removed, and changed entries
sorted by category and path. Each changed value lists the fields that differ. Arrays report the count
of changed elements and a bounded sample of flat, zero-based indices. The
plain-text renderer limits the number of entries while keeping the totals.

.. code-block:: cpp

   #include <snt/dip/comparison.h>

   snt::dip::Comparison comparison{"before.diph5", "after.diph5"};
   std::cout << comparison.render(50);

The default ``Effective`` scope compares persisted value paths, declared and
stored types, shapes, units, and element values. ``Full`` also compares tags,
metadata, options, provenance, settings, and source, trace, and schema
manifests. ``equal()`` means equal within the selected scope; it does not
compare HDF5 bytes or layout. Numeric comparison is exact, with two NaNs
treated as equal. Array examples are controlled by
``ComparisonOptions::max_array_examples``.

.. code-block:: cpp

   snt::dip::ComparisonOptions options;
   options.scope = snt::dip::ComparisonScope::Full;
   options.max_array_examples = 5;
   snt::dip::Comparison comparison{"before.diph5", "after.diph5", options};
   if (!comparison.equal()) {
       for (const auto& difference : comparison.result().differences) {
           // Use the structured differences in your application.
       }
   }

For applications using the command layer, :cpp:class:`snt::api::DIPCompare`
provides the same comparison and text rendering. See also the
:doc:`CLI <../../integrations/cli>`, :doc:`REST <../../integrations/rest>`,
:doc:`Python <../../integrations/python>`, and :doc:`C <../../integrations/c>`
interfaces.

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
