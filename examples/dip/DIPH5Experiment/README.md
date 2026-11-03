# DIPH5 environment, experiment, and results

This example follows one complete DIPH5 v3 workflow. `DIPfile` loads the
`parameters.dip` model. A small Python program produces sample values and a CSV
artifact. The launcher stores the evaluated model and its source bytes, records
the invocation and outcome, registers typed results and the external file, then
verifies the archive.

From the repository root, build `snt` and run:

```sh
bash examples/dip/DIPH5Experiment/run.sh
```

The script creates a new temporary output directory and prints its path. Set
`SNT_EXE` to use another `snt` executable, or pass an empty destination directory
as the first argument. The directory contains:

- `environment.diph5`: evaluated configuration with archived DIPL source bytes.
- `experiment.diph5`: the same configuration plus execution and result records.
- `trajectory.csv`: external result whose size and SHA-256 are registered.
- `values.txt` and `final_value.txt`: small handoff files used by the launcher.

## What the script does

1. `snt dip parse --project ... --save ...` evaluates `DIPfile` and freezes both
   the source bytes and resolved values in `environment.diph5`.
2. Two `snt dip parse --load ... --request ... --value` calls read `steps` and
   `gain` from that archive. The simulation therefore uses the saved inputs.
3. `experiment prepare` copies the frozen configuration into a new experiment
   archive. `experiment start` records the command before the shell invokes it.
4. `simulate.py` writes four scaled values and `trajectory.csv`. While the
   experiment is running, the launcher embeds those values, records a final
   value diagnostic, and registers the CSV as an external artifact.
5. `experiment finalize` seals the result inventory, `finish` stores the exit
   code, and `verify` checks the archive and CSV hashes.

The `experiment add` command stores `trajectory` as a typed numeric array.
`experiment diagnostic` stores an observed final value, while `experiment
attach` registers the CSV without embedding it. `finalize` seals the result
inventory, `finish` records the successful exit code, and `verify` checks the
archive and reachable artifact. The script uses `start`/`finish` because the
launcher registers results between those lifecycle events. It reports the
invocation as `host_reported`; the CLI's `run` command instead observes and
records the process itself, but finishes the experiment before results can be
added.
