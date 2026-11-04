Environment Persistence
=======================

DIP can persist an evaluated environment in the DIPH5 HDF5 format. This is
useful when a DIPL input has already been parsed and evaluated and the
resulting values need to be exchanged with another application, stored for a
later calculation, or inspected by scientific software using HDF5.

DIPH5 v3 stores exact DIPL source bytes, ordered overrides, the resolved
configuration, and optional experiment execution and results. Its root carries
``_DIPH5_Format`` and ``_DIPH5_Schema_Major=3``. SNT3 0.10.0 reads and writes
only v3 archives. DIPH5 v2 and lower are legacy, unsupported formats. The
`historical v2.11 appendix <https://github.com/scinumtools/snt3/blob/main/docs/diph5/specification.md#appendix-legacy-diph5-specification>`_
remains available for interpretation or migration with an older reader.

The recommended filename extension is ``.diph5``; the root attributes identify
the format, not the filename.

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
The saved archive includes exact contributing source bytes and ordered
overrides. Source extraction uses generated filenames within the destination
you choose; it does not write to paths supplied by the archive. See the
`canonical DIPH5 v3 specification <https://github.com/scinumtools/snt3/blob/main/docs/diph5/specification.md>`_
for the on-disk layout and validation rules.

DIPH5 v3 experiments
--------------------

Experiment preparation creates a new file from a saved environment archive.
It refuses an existing destination. Publishing requires hard-link support in
the destination filesystem; lifecycle updates do not carry a durability
guarantee after a system crash.

An experiment adds one ``/execution`` record. The managed CLI path launches
without a shell and records the exact argument array, working directory,
available executable hash, basic system facts, and outcome:

.. code-block:: console

   snt dip parse -i file parameters.dip --save prepared.diph5
   snt dip experiment run --input prepared.diph5 --output run.diph5 -- python simulate.py
   snt dip experiment show --file run.diph5

For an external scheduler, create the record first, then report the command
and its outcome from the host:

.. code-block:: console

   snt dip experiment prepare --input prepared.diph5 --output run.diph5
   snt dip experiment start --file run.diph5 --origin host_reported \
       --role outer_launcher --cwd /work/run -- srun python simulate.py

While a run is marked ``running``, results can be registered in the same
archive. Embedded numbers use their DIPL dtype for storage: signed and unsigned
integers of 8, 16, 32 or 64 bits, ``float32`` and ``float64``, plus ``bool``
and ``char``. The default is ``float64``. Arrays' shapes and optional dimension
names are checked against their values; out-of-range integer values are rejected.
External artifacts remain in their original files and are recorded by path,
size, and SHA-256. Diagnostics distinguish observed statistics from
validation judgments.

.. code-block:: console

   snt dip experiment add --file run.diph5 --name energy --values 42.5 --unit J
   snt dip experiment add --file run.diph5 --name spectrum \
       --values 1,2,3,4 --shape 2,2 --dimensions row,column
   snt dip experiment add --file run.diph5 --name count --dtype uint64 --values 9007199254740993
   snt dip experiment diagnostic --file run.diph5 --name nan_count \
       --values 0 --category observed
   snt dip experiment attach --file run.diph5 --name output --artifact output.nc --format NetCDF
   snt dip experiment results --file run.diph5
   snt dip experiment finalize --file run.diph5
   snt dip experiment finish --file run.diph5 --status completed --exit-code 0
   snt dip experiment verify --file run.diph5

``finish --status completed`` finalizes the inventory automatically if it has
not yet been finalized. An empty inventory is allowed. A failed run retains
its partial results. ``verify`` reports structural and checksum status
separately from external artifact availability. It reads external files but
does not extract or execute them. A missing artifact remains in the inventory
and is reported as unavailable. Finalization and completion recheck reachable
artifact hashes and reject a checksum mismatch. Absolute paths, remote URIs,
and paths resolving outside the archive directory are reported as unavailable
for automatic verification.
The configuration fingerprint is checked separately. For supported values,
the archive stores a canonical digest of the resolved configuration and
``verify`` reports ``pass`` only when it matches. Unsupported values such as
``float128`` leave this check ``unavailable``. The current encoder also marks a
resolved dataset unavailable when its decoded payload exceeds 256 MiB.

Use ``--executable-path``, ``--executable-sha256``, and ``--executable-size``
with ``start`` when the host can report the remote executable's identity.
SNT3 hashes an executable automatically only when it launches the command
itself. With ``--role outer_launcher``, the scientific child's invocation is
recorded as unknown unless the caller supplies a separate observed command.

``import`` records a past terminal outcome when the invocation is unknown.
Import and host-reported operations leave unknown execution timestamps unset;
they record when SNT3 received the report separately. Terminal records cannot
be changed through the normal API. The portable resolved-configuration
fingerprint is verified when the resolved values have portable encodings.

.. _dip-diph5-comparison:

Comparing saved environments
----------------------------

Compare two DIPH5 environments with ``snt dip compare`` or the C++ and Python
``compare_diph5`` functions. The default ``effective`` scope compares resolved
paths, types, shapes, units, and values. The ``full`` scope also reports
changes to tags, metadata, rules, provenance, and persisted source, schema,
and trace manifests. For arrays, the result counts all changed elements and
reports a bounded number of zero-based flat indices. The comparison reads
both archives without modifying either one.

.. code-block:: console

   snt dip compare before.diph5 after.diph5 --scope full --max-array-examples 3

Current limitations
-------------------

A v3 archive requires recoverable source bytes. A loaded v3 archive keeps
those bytes and can be saved again. The portable resolved-configuration
fingerprint is unavailable for unsupported value types.

The resolved-node encoding inside ``/configuration/resolved`` follows the
v2.11 node mapping. This internal encoding does not make v2 files readable by
the 0.10.0 runtime.

Specification
-------------

The PDF contains the canonical v3 specification and the historical v2.11
appendix. You can `open the PDF separately <../../_static/diph5-specification.pdf>`_
or read it below.

* `DIPH5 specification source <https://github.com/scinumtools/snt3/blob/main/docs/diph5/specification.md>`_
* `Historical v2.11 appendix <https://github.com/scinumtools/snt3/blob/main/docs/diph5/specification.md#appendix-legacy-diph5-specification>`_

.. raw:: html

   <iframe
       src="../../_static/diph5-specification.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
