Application adapters
====================

An adapter turns an evaluated ``dip::Environment`` into input files for a
specific downstream program. SNT loads the project or DIPH5 snapshot, calls
``Adapter::plan()``, checks every output path and collision, and writes the
files. The adapter decides which values to use, how to validate them, and the
names and formats of the resulting files. A single adapter can register zero,
one, or many files.

Derive from ``snt::dip::Adapter`` and register text, binary, or streamed
content in ``plan()``. All paths are relative to the output directory.

.. code-block:: cpp

   #include <snt/dip/adapter.h>
   #include <snt/dip/cursor.h>

   class SolverAdapter : public snt::dip::Adapter {
   public:
       void plan(const snt::dip::Environment& env,
                 snt::dip::AdapterContext& context) const override {
           auto steps = env["run.steps"].as<std::int64_t>();
           context.add_text("control.nml", "&run\n steps=" + std::to_string(steps) + "\n/\n");
           context.add_binary("marker.bin", {0x53, 0x4e, 0x54});
           context.add_stream("times.dat", [steps](std::ostream& out) {
               for (std::int64_t i = 0; i < steps; ++i) out << i << '\n';
           });
       }
   };

   auto files = snt::dip::run_adapter_project("DIPfile", SolverAdapter{},
                                               "solver-inputs", "run.diph5");

``run_adapter()`` accepts an existing environment.
``run_adapter_snapshot()`` loads a DIPH5 file before planning. The optional
last argument saves a DIPH5 snapshot alongside the outputs. Each runner
returns the written paths in registration order, followed by the snapshot
when requested.

The runner validates every registered path before calling any stream writer.
It rejects absolute paths, ``.`` and ``..`` components, duplicate paths,
file/directory conflicts, and symbolic links. By default, it also rejects
existing files. Output files are first written in a staging directory, so a
failing writer leaves the destination unchanged.

To regenerate inputs in the same directory, opt in to replacing files
registered for this run:

.. code-block:: cpp

   using snt::dip::ExistingOutputPolicy;

   snt::dip::run_adapter_project("DIPfile", SolverAdapter{}, "solver-inputs",
                                 "run.diph5", ExistingOutputPolicy::ReplaceRegistered);

The optional snapshot follows the same policy. Unregistered files are
preserved. The runner finishes writing all staged outputs before replacing
existing regular files and restores prior files if publication fails.
Python exposes ``ExistingOutputPolicy.ReplaceRegistered`` through the
``existing_output_policy`` argument of all three runner functions. The
default in both languages is ``Reject``. The adapter still decides which
files to register and how to format them.

Adapters can use ordinary environment access, selection, and table inspection
inside ``plan()``. A DIPH5 run can use only the values and provenance retained
in that snapshot. Application-specific validation belongs in the adapter.

See the runnable `AdapterOutputs example
<https://github.com/vrtulka23/scinumtools3/tree/main/examples/dip/AdapterOutputs>`_
for a C++ adapter producing a namelist, binary marker, and streamed data, and
a Python adapter producing JSON and CSV. The :doc:`Python integration guide
<../../integrations/python>` shows the subclassing pattern.

The complete declarations and signatures are in the :doc:`C++ adapter API
<../../api/cpp/dip/adapter>` and :doc:`Python DIP API <../../api/python_dip>`.
