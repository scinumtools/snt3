Traceability and source identities
==================================

DIP assigns internal identifiers while it assembles an environment. They make
diagnostics, generated documentation, and persisted DIPH5 metadata point to a
specific origin without depending on a filesystem path or on the formatting of
the original DIPL text.

Sources
-------

A parser instance has a root identity such as ``DIP0``. Input registration
creates child source identities:

* ``DIP0_STRING0`` for the first :cpp:func:`snt::dip::DIP::add_string` input;
* ``DIP0_FILE0`` for the first :cpp:func:`snt::dip::DIP::add_file` input;
* ``DIP0_SOURCE0`` for a source registered with
  :cpp:func:`snt::dip::DIP::add_source`; and
* ``DIP0_PROJECT0`` for a project manifest registered with
  :cpp:func:`snt::dip::DIP::add_project`.

The numeric suffixes are local counters. They are useful for tracing one
environment, rather than stable public names that should be hard-coded by an
application.

Each source has a :cpp:struct:`snt::dip::SourceInfo` record. It contains its
identity, recorded path, parent source and line, and the SHA-256 fingerprint of
the exact source bytes parsed by DIP. The records are available through
``environment.get_source_manifest()`` and
``environment.get_source_info(name)``.

Registered constructs
---------------------

Units, schemas, and host functions are traceable registrations. Their IDs use
their registering source as a prefix:

.. code-block:: text

   DIP0_UNIT0             # unit added by C++ host code
   DIP0_STRING0_UNIT0     # $unit in the first inline DIPL input
   DIP0_FILE0_SCHEMA0     # $schema in the first DIPL file input
   DIP0_FUNCTION0         # value or node function added by C++ host code

Consequently, ``DIP0_STRING0_UNIT0`` identifies both the unit registration and
the DIPL source that defined it. A host-code registration such as
``dip.add_unit("kmh", "km/h")`` has no DIPL line or source text, so its parent
is the parser root: ``DIP0_UNIT0``.

``environment.get_trace_manifest()`` returns :cpp:struct:`snt::dip::TraceInfo`
records with an ``id``, public ``name``, and ``kind``. The current kinds are
``unit``, ``schema``, ``function_value``, and ``function_nodes``.

DIPH5 persistence
-----------------

DIPH5 2.1 stores source records in ``/_DIPL_Sources`` and registered-construct
records in ``/_DIPL_Trace``. The latter preserves trace IDs, names, and kinds;
it does not recreate custom-unit definitions, schema nodes, or executable host
functions when an environment is loaded. See :doc:`Environment persistence
<persistence>` and the `full DIPH5 specification
<https://github.com/vrtulka23/scinumtools3/blob/main/docs/diph5/specification.md>`_
for the complete on-disk contract.
