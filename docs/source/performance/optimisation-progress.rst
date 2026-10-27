Optimisation progress
=====================

SNT3 performance work is measured in the separate
`snt-bench project <https://github.com/scinumtools/snt-bench>`_ and in the
repository's native ``benchmarks/`` directory. The benchmarks cover different
tasks: parsing DIPL, loading a typed scientific configuration, and operating
on physical quantities. Their timings should be compared within each task,
not treated as one overall speed ranking.

Recorded results
----------------

The following 0.9.11-to-0.9.12 comparisons are from same-host quick
snapshots. They show the combined effect of changes between releases; they do
not isolate the gain from any one code change.

.. list-table::
   :header-rows: 1
   :widths: 24 48 28

   * - Workload
     - Observed change
     - Full method and results
   * - DIPL parser output, 10,000 parameters
     - The DIPL median fell by about 59%. The 0.9.12 median was 141.74 ms,
       compared with 27.28 ms for TOML in the same run.
     - `Parser comparison <https://github.com/scinumtools/snt-bench/tree/main/benchmarks/pyserial>`_
   * - Typed configuration, 1,000 parameters
     - The DIPL median fell by about 88% to 8.615 ms. The same run measured
       2.687 ms for TOML with Pydantic and Pint, and 0.196 ms for JSON with
       Pydantic and Pint.
     - `Typed configuration benchmark <https://github.com/scinumtools/snt-bench/tree/main/benchmarks/typed-config>`_
   * - Python physical-unit operations
     - All 60 SNT3 case medians fell; the median per-case time ratio was 0.81.
       For one-million-element arrays, equal-unit addition fell from 728 to
       381 µs, division from 597 to 344 µs, and temperature conversion from
       337 to 207 µs.
     - `Physical units benchmark <https://github.com/scinumtools/snt-bench/tree/main/benchmarks/pyunits>`_

These are short single-machine runs. The typed configuration benchmark reuses
its Pydantic model but parses DIPL schema declarations on each load, so its
timed setup is not identical across implementations. The parser comparison
returns each implementation's native result; DIPL's result is a typed,
evaluated environment. The benchmark pages record their inputs, versions,
correctness checks, and timing controls.

Changes in the code
-------------------

The largest recurring pattern has been removing repeated work from common
paths:

* DIP maintains an index of value nodes by path and a hashed membership index
  for map keys. This avoids searching growing node and item lists on each
  declaration or modification.
* The DIPL parser reuses compiled regular expressions and checks simple
  characters before attempting expensive matches. Ordinary paths also bypass
  branch-name regex processing when no condition marker is present.
* DIP skips link validation and deferred-node parsing in documents that do not
  need those operations. Schema definitions are accessed by reference during
  expansion rather than copied for every instance.
* PUQ reads compatible arrays from their existing storage, moves owned output
  buffers, and uses single-pass operations for common Float64 conversions and
  arithmetic. This reduces full-array copies and intermediate allocations.
* PUQ avoids constructing uncertainty derivatives when an operation has no
  uncertainty sources to propagate.

These changes interact, so their order here is descriptive, not a ranking of
individual measured speedups. The corresponding native workloads are
documented in `DIP benchmarks <https://github.com/scinumtools/snt3/blob/main/benchmarks/dip/README.md>`_
and `PUQ benchmarks <https://github.com/scinumtools/snt3/blob/main/benchmarks/puq/README.md>`_.

Current limits and next measurements
------------------------------------

Schema-backed maps still do more work per item than explicit typed values:
they expand schema members, resolve paths, record hierarchy entries, and
apply instance values to declarations. The native schema-map benchmark
separates these forms and can be used to check proposed changes without
assuming that schema cloning alone dominates the cost.

For large arrays, same-unit conversion still produces independent result
storage. In the recorded one-million-element case, SNT3 took 236 µs while
Pint took 4.24 µs and shared its public NumPy storage. That difference is
partly about result ownership, so any change to this path needs both a speed
comparison and an explicit decision about aliasing semantics.

To reproduce the native tests, run ``./setup.sh --benchmark dip`` or
``./setup.sh --benchmark puq`` from the SNT3 repository root. The
`snt-bench instructions <https://github.com/scinumtools/snt-bench>`_ describe
the corresponding Python comparisons and local-source runs. Use the same
Python interpreter, dependencies, hardware, and workload settings for
before-and-after measurements; repeat runs before claiming a stable gain.
