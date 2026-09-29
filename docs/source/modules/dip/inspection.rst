Inspecting and reloading environments
=====================================

The read-only ``<snt/dip/inspection.h>`` API collects facts already retained
by an evaluated environment. ``inspect_value`` returns an owned typed value,
shape, units, metadata, tags, declaration and override locations, and schema
information. ``inspect_values`` returns all evaluated values in environment
order with their full paths.

.. code-block:: cpp

   #include <snt/dip/inspection.h>

   auto env = snt::dip::open_artifact("DIPfile");
   auto speed = snt::dip::inspect_value(env, "physics.speed");
   auto shape = speed.shape;
   auto source = speed.declaration_location;
   if (speed.override_location) {
       auto replacement = *speed.override_location;
   }

This is the current declaration and applied override, when present. It is
not a history of every modification. The ``provenance`` member retains the
existing source manifest details; ``contributing_schema`` identifies a schema
that supplied the value when known. A loaded DIPH5 environment contains only
provenance retained in the snapshot.

``detect_artifact(path)`` classifies conventional names: ``DIPfile``,
``.dip`` or ``.dipl``, ``.dipt``, and ``.diph5``. It does not validate file
contents. ``open_artifact(path)`` loads a project, DIPL file, or DIPH5 snapshot.
``.dipt`` is currently table input used within DIPL, so it cannot be opened
as an independent environment.

To refresh after an external edit, use ``reload_artifact``. It builds a fresh
environment and replaces the current one only when parsing or loading succeeds:

.. code-block:: cpp

   snt::dip::reload_artifact(env, "DIPfile");

For in-memory arrays, ``read_value_slice`` accepts zero-based inclusive
ranges and returns a typed VAL value containing the selected elements:

.. code-block:: cpp

   auto values = snt::dip::read_value_slice(env, "samples", {{10, 19}});

This slices an already evaluated value. DIPH5 loading remains eager; this API
does not provide disk-backed lazy reads.
