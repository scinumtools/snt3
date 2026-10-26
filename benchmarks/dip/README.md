# DIPL parsing and Python extraction benchmark

The suite measures DIPL in both native C++ and the Python binding. The native
`benchmark-dip` target separates:

- `BM_DipAddString`: construction and registration of source text.
- `BM_DipParseOnly`: `DIP::parse()` with parser construction and source
  registration paused outside the timed region.
- `BM_DipParseEndToEnd`: construction, source registration, and parsing.
- `BM_DipSelectSnapshots`: C++ selection including independent node clones.
- `BM_DipReadOwnedNodes`: native iteration over the environment-owned nodes.

The native benchmark validates parsed parameter values before timing. The
Python `python_parse_benchmark.py` separates three costs combined in an
end-to-end DIPL-to-dictionary timing:

- `parse_evaluate`: construct `DIP`, add the already-generated source string,
  and parse/evaluate it into an `Environment`.
- `select_snapshot_clones`: call `Environment.select("?")`. Selection returns
  independent node snapshots, so this isolates the selection and clone cost.
- `extract_snapshot_values`: convert an already-selected snapshot list to a
  Python dictionary, isolating Python name/value access and conversion.
- `snapshot_to_dict`: perform the current end-to-end selection-and-extraction
  route used by the pyserial benchmark.
- `node_list_to_dict`: extract the same values directly from the environment's
  owned node list, without first cloning snapshots.

Every size is correctness-checked before timing: snapshot and node-list
extraction must reproduce the same flat mapping of fully qualified DIPL paths
to Python scalar values. Source generation and the correctness checks are
outside the timed regions. The benchmark uses the same nested scalar parameter
shape as `snt-paper/benchmarks/pyserial`.

The `benchmark-dip-schema-map` target isolates typed configuration structure
costs at 10, 100, 1,000, and 10,000 values. It reports:

- `BM_DipSchemaDeclarationsOnly`: parse two schemas without creating instances.
- `BM_DipTypedFlat`: create typed scalar values beneath ordinary groups.
- `BM_DipTypedExplicitMap`: create keyed map items and typed child values
  directly, without schemas.
- `BM_DipTypedSchemaMap`: expand a map item schema and apply each value as an
  instance modification.
- `BM_DipTypedSchemaMapCondition`: the same schema-backed map with a condition
  evaluated for every value.
- `BM_DipTypedSchemaMapDefault`: expand schema-backed items with a default
  value and no instance modifications.
- `BM_DipTypedSchemaMapDefaultOverride`: use that same default schema, then
  modify each item value.

All timed cases parse a prebuilt input; source generation and a value-node count
check are outside timing. Payload bytes and values processed are reported too.
These cases separate schema declaration setup, flat node creation, map and
hierarchy work, schema expansion, and per-value condition checks. The direct
map and schema-backed map forms are diagnostic representations: the latter
retains schema-based declarations and applies values as modifications, while
the former declares each typed child with its value. Compare timings as clues
about where work is spent, not as a claim that the two DIPL models are
semantically interchangeable.

## Run

Run both the native and Python suites from the repository root with:

```sh
./setup.sh --benchmark dip
```

This builds `benchmark-dip`, `benchmark-dip-schema-map`, and the Python DIP binding. Google Benchmark's
colored report is retained, and Python per-case timing samples are written to
`build/dip-python-benchmark.csv`. Native C++ runs are recorded in the ignored
`benchmarks/dip/results/history.json` and can be compared with the existing
`benchmarks/puq/benchmark_history.py compare --history ...` command.
The setup helper uses a quick profile (three C++ repetitions and three Python
samples) so large parse cases do not make routine runs too slow.

The `dip` suite also builds and records `benchmark-dip-schema-map`. Run it
directly to focus on these cases:

```sh
build/bin/benchmark-dip-schema-map --benchmark_repetitions=5 --benchmark_min_time=0.05s
```

To run only the Python phases, first build the binding with DIP enabled, then
use:

```sh
cmake -S . -B build -DENABLE_BINDING_PYTHON=ON -DENABLE_DIP=ON -DENABLE_DIP_PYBIND=ON
cmake --build build --target _snt
PYTHONPATH=build/python .venv/bin/python benchmarks/dip/python_parse_benchmark.py \
  --sizes 1000 10000 --samples 7 --target-ms 100 \
  --output benchmarks/dip/results/local.csv
```

Use a Release build and repeat independent processes under similar machine
load for performance conclusions. `node_list_to_dict` measures a lower-copy
extraction route through the existing Python API; it is still Python-level
per-node access, so it is not a parser-only measurement.

For a higher-confidence native run, increase Google Benchmark repetitions and
minimum time directly:

```sh
build/bin/benchmark-dip --benchmark_repetitions=7 --benchmark_min_time=0.1s
```
