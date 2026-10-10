Application adapters
====================

An adapter turns an evaluated ``dip::Environment`` into input files for a
specific downstream program. SNT loads the project or DIPH5 snapshot, resolves
the adapter's output plan, checks every output path and collision, and writes the
files. The adapter decides which values to use, how to validate them, and the
names and formats of the resulting files. A single adapter can register zero,
one, or many files.

Adapter-owned output plans
--------------------------

An adapter may declare native mappings before writing files. Override
``describe_outputs(env, outputs)`` to add mappings from real parameter paths
with ``outputs.add_node(...)``, or from typed calculated values with
``outputs.add_value(...)``. Each mapping has a stable ID, target, native key,
active decision, and optional rule label, dependencies, and declaration origin.
Use ``replace_node`` or ``replace_value`` when a profile intentionally changes
a mapping with the same ID. Accidental duplicate IDs and duplicate active
native keys within a target are rejected. Tags may help the adapter discover
parameter groups; they do not define output policy.

Override ``plan_resolved(env, outputs, context)`` to format the selected
mappings and register native files. Existing adapters implementing only
``plan(env, context)`` continue to work. For example:

.. code-block:: python

   from scinumtools3.dip import Adapter

   class SolverAdapter(Adapter):
       def describe_outputs(self, env, outputs):
           outputs.add_node(env, "run.steps", "settings", "Steps", "run.steps",
                            active=False, origin="base")
           outputs.replace_node(env, "run.steps", "settings", "Steps", "run.steps",
                                active=True, origin="profile")

       def plan_resolved(self, env, outputs, context):
           lines = [f"{item.key}={item.value}" for item in outputs.select("settings", True)]
           context.add_text("control.in", "\n".join(lines) + "\n")

The adapter evaluates its policy against the final environment and records
the resulting typed values and decisions in the plan. A DIPH5 snapshot written
by an adapter run stores that resolved plan. A later
``run_adapter_snapshot()`` passes the saved plan to ``plan_resolved`` without
rerunning ``describe_outputs``; this preserves profile choices even when the
original profile files are unavailable. The plan is inspectable through
``Environment.output_plan`` after loading the snapshot. Derived mappings
remain outside the parameter tree and Parameter guide.
Call ``resolve_output_plan(env, adapter)`` to inspect the same validated plan
before writing any native files.

For AI-assisted setup of an established simulation code, DIPL can describe run
settings and references to initial-condition inputs. An adapter can check
code-specific requirements and generate the program's normal parameter file.
These checks catch interface errors; they do not establish that the setup is
physically appropriate.

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

To also remove files generated by an earlier run that are no longer planned,
use ``ExistingOutputPolicy::SyncRegistered`` in C++ or
``ExistingOutputPolicy.SyncRegistered`` in Python. This mode writes
``.snt-adapter-manifest`` in the output directory to record the registered
paths, including the optional snapshot. On the next sync run, it replaces
current registered files and removes only stale paths from that manifest.
Unregistered files remain untouched. The manifest and file changes are staged
and restored together if publication fails. The first sync run has no prior
record, so it does not remove files left by earlier ``ReplaceRegistered`` runs.

Adapters can use ordinary environment access, selection, and table inspection
inside ``plan()``. A DIPH5 run can use only the values and provenance retained
in that snapshot. Application-specific validation belongs in the adapter.

See the runnable `AdapterOutputs example
<https://github.com/scinumtools/snt3/tree/main/examples/dip/AdapterOutputs>`_
for a C++ adapter producing a namelist, binary marker, and streamed data, and
a Python adapter producing JSON and CSV. The :doc:`Python integration guide
<../../integrations/python>` shows the subclassing pattern.

The complete declarations and signatures are in the :doc:`C++ adapter API
<../../api/cpp/dip/adapter>` and :doc:`Python DIP API <../../api/python_dip>`.
