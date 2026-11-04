DIPH5 — Scientific Contract
===========================

DIPH5 is SNT3's HDF5 format for preserving the scientific parameters of a
calculation. A DIPL project defines the parameter model; evaluating it produces
values that can be saved as a DIPH5 file. The file keeps the contributing source
text, ordered overrides, resolved configuration, and provenance together, so a
reader can inspect both the effective values and how they were obtained.

The same format can hold an experiment record: one frozen configuration, at most
one execution attempt, and its registered results. Separate runs use separate
files. An environment-only DIPH5 file has no execution or results; an experiment
file adds an execution record and may include results.

File structure
--------------

The four principal groups in a DIPH5 v3 file have distinct roles:

.. list-table::
   :header-rows: 1
   :widths: 22 78

   * - Group
     - Contents
   * - ``/model``
     - Contributing DIPL sources, source registrations, and their manifest.
   * - ``/configuration``
     - Ordered overrides, resolved parameter values, and configuration provenance.
   * - ``/execution``
     - The recorded command, software, runtime, and outcome of one attempt.
       Present only in an experiment file.
   * - ``/results``
     - Registered numerical datasets, external artifacts, and diagnostics.
       Optional in an experiment file.

For example, this command evaluates a DIPL source and saves its configuration:

.. code-block:: console

   snt dip parse -i file parameters.dip --save prepared.diph5

A later run can use that saved configuration and create an experiment file:

.. code-block:: console

   snt dip experiment run --input prepared.diph5 --output run.diph5 -- python simulate.py
   snt dip experiment show --file run.diph5
   snt dip experiment verify --file run.diph5

The source archive supports inspection and emergency reconstruction of the
contributing DIPL text. The resolved configuration remains the saved result of
evaluation; changing a source file later does not change values already stored
in the DIPH5 file.

Where to go next
----------------

Read about :doc:`DIPL <../dipl/index>` for the parameter language and
:doc:`DIPfile projects <../modules/dip/projects>` for multi-file inputs. The
:doc:`persistence guide <../modules/dip/persistence>` covers saving, loading,
experiment registration, and verification through SNT3's interfaces.

DIPH5 v3 is the canonical format from SNT3 0.10.0. Earlier DIPH5 versions are
legacy formats and are not read by the current runtime. The
`normative DIPH5 specification <https://github.com/scinumtools/snt3/blob/main/docs/diph5/specification.md>`_
includes a historical v2.11 appendix for interpretation and migration. You can
also `open the specification PDF <../_static/diph5-specification.pdf>`_ or read
it below.

.. raw:: html

   <iframe
       src="../_static/diph5-specification.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
