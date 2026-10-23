#!/usr/bin/env python3
"""Measure NumPy-to-PUQ construction and PUQ array operations separately."""

from __future__ import annotations

import argparse
import gc
import json
import platform
import statistics
import sys
import time
from pathlib import Path

import numpy as np

import scinumtools3
from scinumtools3.puq import Quantity


def cmake_settings(module_path: Path, explicit_cache: Path | None) -> dict[str, str]:
    """Include available compiler/build details for before-and-after runs."""
    cache = explicit_cache or module_path.parent.parent.parent / "CMakeCache.txt"
    if not cache.is_file():
        return {}
    keys = {
        "CMAKE_BUILD_TYPE",
        "CMAKE_GENERATOR",
        "CMAKE_CXX_COMPILER",
        "CMAKE_CXX_FLAGS",
        "CMAKE_CXX_FLAGS_RELEASE",
        "CMAKE_OSX_ARCHITECTURES",
    }
    settings = {"cache": str(cache.resolve())}
    for line in cache.read_text(encoding="utf-8").splitlines():
        if line.startswith(("#", "//")) or "=" not in line:
            continue
        key_and_type, value = line.split("=", 1)
        key = key_and_type.split(":", 1)[0]
        if key in keys:
            settings[key] = value
    return settings


def check_quantity(quantity: Quantity, expected: np.ndarray, uncertainty: np.ndarray | None = None) -> None:
    actual = quantity.value(numpy=True)
    if actual.shape != expected.shape:
        raise AssertionError(f"shape {actual.shape} != {expected.shape}")
    np.testing.assert_allclose(actual, expected, rtol=1e-12, atol=1e-12)
    if uncertainty is not None:
        actual_uncertainty = quantity.uncertainty(numpy=True)
        if actual_uncertainty.shape != uncertainty.shape:
            raise AssertionError(f"uncertainty shape {actual_uncertainty.shape} != {uncertainty.shape}")
        np.testing.assert_allclose(actual_uncertainty, uncertainty, rtol=1e-12, atol=1e-12)


def measure(operation, warmups: int, samples: int, iterations: int) -> dict:
    for _ in range(warmups):
        operation()
    timings = []
    for _ in range(samples):
        gc.collect()
        gc.disable()
        try:
            start = time.perf_counter_ns()
            for _ in range(iterations):
                result = operation()
            elapsed = time.perf_counter_ns() - start
            _ = result  # Keep the last result live to the end of the sample.
        finally:
            gc.enable()
        timings.append(elapsed / iterations)
    median = statistics.median(timings)
    return {
        "iterations_per_sample": iterations,
        "samples_ns_per_operation": timings,
        "median_ns_per_operation": median,
        "median_absolute_deviation_ns": statistics.median(abs(t - median) for t in timings),
    }


def run_size(size: int, warmups: int, samples: int, iterations: int | None) -> dict:
    values = np.linspace(0.0, 30.0, size, dtype=np.float64)
    strided_storage = np.empty(size * 2, dtype=np.float64)
    strided_storage[::2] = values
    strided = strided_storage[::2]
    float32_values = values.astype(np.float32)
    errors = np.full(size, 0.25, dtype=np.float64)

    source_m = Quantity(values, "m")
    source_cel = Quantity(values, "Cel")
    source_dbm = Quantity(values, "dBm")
    source_uncertain = Quantity(values, errors, "m")
    other_m = Quantity(np.full(size, 2.0), "m")

    cases = {
        "construct_contiguous_m": (lambda: Quantity(values, "m"), values, None),
        "construct_strided_m": (lambda: Quantity(strided, "m"), values, None),
        "construct_float32_m": (lambda: Quantity(float32_values, "m"), float32_values.astype(np.float64), None),
        "construct_uncertain_m": (lambda: Quantity(values, errors, "m"), values, errors),
        "convert_same_m_to_m": (lambda: source_m.convert("m"), values, None),
        "convert_linear_m_to_mm": (lambda: source_m.convert("mm"), values * 1000.0, None),
        "convert_offset_Cel_to_K": (lambda: source_cel.convert("K"), values + 273.15, None),
        "convert_logarithmic_dBm_to_mW": (lambda: source_dbm.convert("mW"), 10.0 ** (values / 10.0), None),
        "convert_uncertain_m_to_mm": (lambda: source_uncertain.convert("mm"), values * 1000.0, errors * 1000.0),
        "add_arrays_m": (lambda: source_m + other_m, values + 2.0, None),
    }

    result = {}
    count = iterations or max(10, min(1000, 1_000_000 // size))
    for name, (operation, expected, uncertainty) in cases.items():
        check_quantity(operation(), expected, uncertainty)
        result[name] = measure(operation, warmups, samples, count)

    # Check multidimensional shape preservation with a noncontiguous input.
    matrix = np.arange(24.0).reshape(4, 6)[:, ::2]
    check_quantity(Quantity(matrix, "m"), matrix)

    try:
        source_m.convert("s")
    except Exception:
        pass
    else:
        raise AssertionError("conversion from metres to seconds unexpectedly succeeded")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sizes", default="1000,10000,100000", help="Comma-separated array lengths")
    parser.add_argument("--samples", type=int, default=7, help="Independent timing samples per case")
    parser.add_argument("--warmups", type=int, default=3, help="Untimed calls before each case")
    parser.add_argument("--iterations", type=int, help="Operations per sample (default scales with array length)")
    parser.add_argument("--cmake-cache", type=Path, help="CMakeCache.txt to record build settings")
    parser.add_argument("--build-notes", default="", help="Additional compiler or package-build details")
    parser.add_argument("--output", type=Path, help="Write JSON results to this path instead of stdout")
    args = parser.parse_args()

    try:
        sizes = [int(value) for value in args.sizes.split(",")]
    except ValueError:
        parser.error("--sizes must contain comma-separated integers")
    if not sizes or any(size < 1 for size in sizes):
        parser.error("--sizes must contain positive integers")
    if args.samples < 1 or args.warmups < 0 or (args.iterations is not None and args.iterations < 1):
        parser.error("samples and iterations must be positive; warmups must be nonnegative")

    module_path = Path(scinumtools3.__file__).resolve()
    report = {
        "metadata": {
            "scinumtools3_version": scinumtools3.__version__,
            "scinumtools3_module": str(module_path),
            "numpy_version": np.__version__,
            "python_version": sys.version,
            "platform": platform.platform(),
            "machine": platform.machine(),
            "processor": platform.processor(),
            "build_settings": cmake_settings(module_path, args.cmake_cache),
            "build_notes": args.build_notes,
            "clock": "time.perf_counter_ns",
            "warmup_calls_per_case": args.warmups,
            "samples_per_case": args.samples,
        },
        "sizes": {str(size): run_size(size, args.warmups, args.samples, args.iterations) for size in sizes},
    }
    output = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(output, encoding="utf-8")
        print(f"Wrote {args.output}")
        for size, cases in report["sizes"].items():
            for name, stats in cases.items():
                print(f"{size:>7} {name:<35} {stats['median_ns_per_operation'] / 1e6:>9.3f} ms/op")
    else:
        sys.stdout.write(output)


if __name__ == "__main__":
    main()
