# Adapter outputs

This DIPfile defines a step count and a time interval. Two independent adapters
turn the evaluated values into inputs for different downstream programs. The
C++ adapter writes a Fortran namelist, a binary marker, streamed time data,
and an optional DIPH5 snapshot. The Python adapter writes JSON and streamed
CSV. The formats and filenames belong to the adapters, not to SNT.

From the repository root, after building the C++ targets and Python binding:

```sh
build/bin/ExampleDipAdapter
PYTHONPATH=build/python python examples/dip/AdapterOutputs/convert.py
```

Pass a DIPfile path and output directory as positional arguments to the C++
program. The Python script accepts `--project` and `--output`. Both runners
reject existing output files by default. Code that needs to regenerate inputs
in the same directory can pass `ExistingOutputPolicy::ReplaceRegistered` in
C++, or `existing_output_policy=ExistingOutputPolicy.ReplaceRegistered` in
Python. Only files registered for that run are replaced. `run_adapter_snapshot()`
can regenerate the same inputs from the C++ example's `run.diph5` file.
Use `ExistingOutputPolicy::SyncRegistered` (or
`ExistingOutputPolicy.SyncRegistered` in Python) to also remove files recorded
by a previous sync run when they are no longer registered. The runner stores
that list in `.snt-adapter-manifest` and preserves unregistered files.
