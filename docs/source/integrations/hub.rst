SNT Hub
=======

.. _cli-hub:

Pinned code examples with SNT Hub
---------------------------------

SNT Hub connects a project in the published `Hub catalogue
<https://scinumtools.github.io/snt-hub/#registry>`_ to a local, pinned
workspace. You can inspect its source and DIPL model, prepare native inputs
from an example, and use optional build or local run recipes when the project
provides them. ``setup`` prepares files; it does not build or run the solver.

Quick path
~~~~~~~~~~

.. code-block:: bash

   mkdir study && cd study
   snt hub list              # Find a project ID
   snt hub fetch PROJECT     # Create a pinned workspace here
   snt hub examples          # Find an example ID and its capability
   snt hub info              # Check the pinned origin and revisions
   snt hub setup EXAMPLE     # Prepare runs/EXAMPLE

Replace ``PROJECT`` with an ID from ``list`` and ``EXAMPLE`` with a name from
``examples``. A ``complete`` example produces the inputs required by its
recipe. Other examples require ``--inputs-only``; see :ref:`Prepare an example
<cli-hub-setup>` below.

Discover and pin a project
~~~~~~~~~~~~~~~~~~~~~~~~~~

``snt hub list`` reads the live catalogue on each invocation; it needs network
access and does not use a local cache. Its table shows the first 10 characters
of each pinned source revision; the full revision remains in the catalogue.
``fetch`` also needs Git and network
access. It writes into the current directory unless you pass ``--dir PATH``.
You can pin a particular catalogue revision with ``--revision HUB_SHA``; use
the full SHA currently advertised by the catalogue.

A fetched workspace contains:

* ``source/`` — the pinned original project source.
* ``dipl/`` — editable DIPL model and adapter files.
* ``.snthub/lock.json`` — the Hub and source revisions and their origin.

``fetch`` refuses existing workspace files. If the workspace already matches
the requested project and revision, it reuses it without replacing local
edits. Run ``snt hub info`` to inspect its lock before using the adapter with
your data. ``examples``, ``info``, ``setup``, ``build``, and ``run`` find the
workspace by searching upward for ``.snthub/lock.json``. Use
``--workspace DIR`` with any of them to select a workspace explicitly.

.. _cli-hub-setup:

Prepare an example
~~~~~~~~~~~~~~~~~~

``snt hub examples`` lists each recipe with a capability:

* ``complete`` — ``snt hub setup EXAMPLE`` prepares the recipe's required
  assets, native input files, and an ``environment.diph5`` snapshot.
* ``native-inputs-only`` or ``requires-external-data`` — pass
  ``--inputs-only`` explicitly. The prepared files still need additional
  inputs before they can run.

Setup writes to ``runs/EXAMPLE`` by default. Use ``--output DIR`` for a
different directory inside the workspace. It refuses an existing destination
and publishes the directory only after the adapter succeeds. Python 3 with
``venv`` and ``pip`` is needed when ``setup``, ``build``, or ``run`` first
provisions the workspace's adapter runtime.

To change values for one prepared run, write bare DIPL override assignments
to a file. The paths must exist in the selected DIPL model; these names only
illustrate the syntax:

.. code-block:: text

   simulation.steps = 100
   solver.tolerance = 1e-6

Then pass that file to setup, without a ``$override`` wrapper:

.. code-block:: bash

   snt hub setup EXAMPLE --override-file tuning.dip \
       --output ./runs/tuned_example

The adapter evaluates the override with DIPL. An accepted file is copied to
``input-overrides.dip`` and its SHA-256 is recorded in ``setup-lock.json``.
A complete recipe with a fixed data generator may allow only approved targets.
Rejected targets do not publish the requested directory.

Build and run when supported
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``snt hub build`` and ``snt hub run`` are available only if the pinned project
declares the corresponding reviewed capability. A project may offer either
one independently. Undeclared capabilities are reported before an adapter
runtime is created; use the project's upstream instructions if it has no
Hub build recipe. Build profiles are defined by the project.

.. code-block:: bash

   snt hub build --setup runs/EXAMPLE --profile local
   cd runs/EXAMPLE
   snt hub run
   # Or select an existing executable:
   snt hub run --executable /path/to/solver

Pass ``--setup DIR`` to ``build`` or ``run`` when working outside the prepared
run directory. A build recipe that requires a setup checks its
``setup-lock.json``. It may accept an inputs-only setup if that setup contains
the required build configuration. A successful build writes ``build/PROFILE``
and ``build-lock.json`` with the source revision, optional setup-lock digest,
adapter build details, and executable SHA-256.

``run`` requires a ``complete`` setup. It checks a matching build lock, or
hashes the executable named with ``--executable``. It records the executable
identity and exit status in the setup directory's ``run-lock.json``, even if
the solver fails. A second run in the same directory is refused until a rerun
policy is defined. The project adapter chooses the build and solver commands;
SNT does not infer them from the project name.

