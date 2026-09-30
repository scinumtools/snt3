DIPH5 comparison
================

Compare two evaluated DIPH5 snapshots with :cpp:func:`snt::dip::compare_diph5`.
The result contains added, removed, and changed entries sorted by category and
path. Each changed value lists the fields that differ. Arrays report the count
of changed elements and a bounded sample of flat, zero-based indices. The
plain-text renderer limits the number of entries while keeping the totals.

.. code-block:: cpp

   #include <snt/dip/comparison.h>

   auto result = snt::dip::compare_diph5("before.diph5", "after.diph5");
   std::cout << snt::dip::render_comparison(result, 50);

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
   auto result = snt::dip::compare_diph5("before.diph5", "after.diph5", options);

For applications using the command layer, :cpp:class:`snt::api::DIPCompare`
provides the same comparison and text rendering. See also the
:doc:`CLI <../../integrations/cli>`, :doc:`REST <../../integrations/rest>`,
:doc:`Python <../../integrations/python>`, and :doc:`C <../../integrations/c>`
interfaces.
