# PUQ benchmarks

## Scalar operations

`benchmark-puq-scalar` records the current cost of scalar `Result` construction,
arithmetic, uncertainty propagation, and unit-aware `Quantity` operations.
The operands and unit expressions are prepared before the timed loops. An
array-add case provides a regression control for future scalar optimizations.

Build and run it with Google Benchmark enabled:

```sh
cmake -S . -B build-bench -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_EXEC_BENCHMARKS=ON -DENABLE_UNIT_TESTS=OFF \
  -DENABLE_BINDING_PYTHON=OFF -DENABLE_BINDING_C=OFF \
  -DENABLE_EXEC_APPS=OFF -DENABLE_EXEC_EXAMPLES=OFF \
  -DENABLE_DIP=OFF -DENABLE_MAT=OFF -DENABLE_API=OFF
cmake --build build-bench --target benchmark-puq-scalar
build-bench/bin/benchmark-puq-scalar
```

The build uses an installed Google Benchmark package when available, or fetches
the project's pinned version. Compare runs made with the same compiler, build
type, machine, and benchmark settings. The `Result` cases isolate numerical
storage and arithmetic; the `Quantity` cases also include unit handling.

## Array construction and conversion

`benchmark-puq-array` measures C++ array storage construction, `Quantity`
construction, array addition, and conversion of 1,000, 10,000, and 100,000
values. Conversion cases cover same units (`m` to `m`), linear scaling (`m`
to `mm`), offset (`Cel` to `K`), logarithmic (`dBm` to `mW`), and uncertainty
propagation. Input quantities are prepared outside each timed loop. Use this
suite to measure the core independently of Python bindings.

```sh
cmake --build build-bench --target benchmark-puq-array
build-bench/bin/benchmark-puq-array --benchmark_repetitions=7 \
  --benchmark_out=puq-array-cpp.json --benchmark_out_format=json
```

`python_array_benchmark.py` measures SciNumTools3's Python/NumPy path alone.
It separates array construction from conversion, covers contiguous, strided,
and `float32` inputs, and checks values and shapes before timing. Its JSON
output records raw samples, medians, variability, package versions, platform,
and available CMake build settings:

```sh
PYTHONPATH=build/python .venv/bin/python benchmarks/puq/python_array_benchmark.py \
  --output puq-array-python.json
```

Use a Release build and the same Python, NumPy, machine, and run settings for
before-and-after measurements. Use the paper repository for cross-library
comparisons. The C++ benchmark isolates the core path; these measurements are
not timings of downstream applications.

## Keeping and comparing runs

`benchmark_history.py` runs the available C++ benchmarks and the Python array
benchmark, then stores their raw JSON results with a short Git commit hash. It
keeps the newest 20 runs by default. The history is written to the ignored
`benchmarks/puq/results/history.json`, so it stays local to the checkout.
Google Benchmark's colored console table remains visible for every C++ suite.

```sh
cmake --build build-bench --target benchmark-puq-scalar benchmark-puq-array
.venv/bin/python benchmarks/puq/benchmark_history.py record \
  --cpp-benchmark build-bench/bin/benchmark-puq-scalar \
  --cpp-benchmark build-bench/bin/benchmark-puq-array \
  --python-path build/python
.venv/bin/python benchmarks/puq/benchmark_history.py compare
```

Run `compare --baseline 3 --target 1` to compare the third newest run with the
latest run; index `1` always means newest. Use `record --keep 30` to change the
retention limit, or `record --skip-python` for a C++-only run. If the benchmark
executables live elsewhere, pass their paths with `--cpp-benchmark` as shown.
The repository helper also accepts a suite selector, for example
`./setup.sh --benchmark array`, `./setup.sh --benchmark scalar`,
`./setup.sh --benchmark python`, or `./setup.sh --benchmark all`. It builds the
selected target(s), records the results in the same history, and defaults to
the original `puq` benchmark when `--benchmark` has no suite argument.
