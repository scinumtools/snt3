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
refuse to overwrite existing files, so use a fresh output directory for another
run. `run_adapter_snapshot()` can regenerate the same inputs from the C++
example's `run.diph5` file.
