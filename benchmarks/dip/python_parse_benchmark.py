#!/usr/bin/env python3
"""Separate DIPL parse/evaluation time from Python node extraction overhead."""

from __future__ import annotations

import argparse
import csv
import gc
import platform
import statistics
import time
from pathlib import Path

import scinumtools3
from scinumtools3.dip import DIP


def make_document(count: int) -> tuple[str, dict[str, object]]:
    """Build a deterministic scalar-parameter document and its flat expected values."""
    values: dict[str, object] = {
        "experiment.name": "serialization benchmark",
        "experiment.enabled": True,
        "experiment.iterations": 1000,
        "experiment.timestep": 0.00025,
    }
    lines = [
        "experiment",
        '  name str = "serialization benchmark"',
        "  enabled bool = true",
        "  iterations int = 1000",
        "  timestep float = 0.00025",
        "  parameters",
    ]
    for index in range(count):
        name = f"p{index:05d}"
        value = (index % 1000) / 10.0
        lines.append(f"    {name} float = {value!r}")
        values[f"experiment.parameters.{name}"] = value
    return "\n".join(lines), values


def parse_document(payload: str):
    """Parse and evaluate one already-built DIPL document."""
    parser = DIP()
    parser.add_string(payload)
    return parser.parse()


def from_snapshots(environment) -> dict[str, object]:
    """Extract values through Environment.select(), which returns node snapshots."""
    return {node.name: node.value for node in environment.select("?")}


def from_node_list(environment) -> dict[str, object]:
    """Read the environment-owned node list without cloning node snapshots."""
    nodes = environment.nodes
    return {nodes[index].name: nodes[index].value for index in range(nodes.size())}


def measure(operation, warmups: int, samples: int, target_ms: float) -> dict[str, object]:
    """Calibrate iterations, then report per-call samples and robust summaries."""
    for _ in range(warmups):
        operation()

    iterations = 1
    while True:
        start = time.perf_counter_ns()
        for _ in range(iterations):
            result = operation()
        elapsed_ns = time.perf_counter_ns() - start
        if elapsed_ns >= target_ms * 1e6 or iterations >= 10_000_000:
            break
        iterations = max(iterations + 1, int(iterations * target_ms * 1e6 / max(elapsed_ns, 1)))

    timings = []
    for _ in range(samples):
        gc.collect()
        gc.disable()
        try:
            start = time.perf_counter_ns()
            for _ in range(iterations):
                result = operation()
            elapsed_ns = time.perf_counter_ns() - start
            _ = result
        finally:
            gc.enable()
        timings.append(elapsed_ns / iterations)

    median = statistics.median(timings)
    return {
        "iterations_per_sample": iterations,
        "samples_ns_per_operation": timings,
        "median_ns_per_operation": median,
        "iqr_ns_per_operation": statistics.quantiles(timings, n=4)[2]
        - statistics.quantiles(timings, n=4)[0],
    }


def run_size(count: int, warmups: int, samples: int, target_ms: float) -> list[dict[str, object]]:
    payload, expected = make_document(count)
    environment = parse_document(payload)

    # Correctness checks run before timing. Both extraction paths must recover
    # exactly the same typed scalar values from the evaluated environment.
    snapshots = environment.select("?")
    snapshot_values = {node.name: node.value for node in snapshots}
    node_values = from_node_list(environment)
    if snapshot_values != expected:
        raise AssertionError("snapshot extraction did not reproduce the expected values")
    if node_values != expected:
        raise AssertionError("node-list extraction did not reproduce the expected values")

    cases = {
        "parse_evaluate": lambda: parse_document(payload),
        "select_snapshot_clones": lambda: environment.select("?"),
        "extract_snapshot_values": lambda: {node.name: node.value for node in snapshots},
        "snapshot_to_dict": lambda: from_snapshots(environment),
        "node_list_to_dict": lambda: from_node_list(environment),
    }
    rows = []
    for case, operation in cases.items():
        result = measure(operation, warmups, samples, target_ms)
        rows.append(
            {
                "case": case,
                "parameters": count,
                "payload_bytes": len(payload.encode("utf-8")),
                "median_ns": result["median_ns_per_operation"],
                "iqr_ns": result["iqr_ns_per_operation"],
                "iterations_per_sample": result["iterations_per_sample"],
                "samples_ns": ";".join(str(round(value)) for value in result["samples_ns_per_operation"]),
                "python_version": platform.python_version(),
                "implementation": platform.python_implementation(),
                "platform": platform.platform(),
                "scinumtools3_version": scinumtools3.__version__,
                "status": "PASS",
            }
        )
    return rows


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sizes", nargs="+", type=int, default=[10, 100, 1_000, 10_000])
    parser.add_argument("--samples", type=int, default=5)
    parser.add_argument("--warmups", type=int, default=2)
    parser.add_argument("--target-ms", type=float, default=50)
    parser.add_argument("--output", type=Path, help="Optional CSV output path")
    args = parser.parse_args()
    if any(size < 1 for size in args.sizes) or len(set(args.sizes)) != len(args.sizes):
        parser.error("sizes must be positive and unique")
    if args.samples < 2 or args.warmups < 0 or args.target_ms <= 0:
        parser.error("samples must be >= 2, warmups >= 0, and target-ms > 0")

    rows = []
    for count in args.sizes:
        rows.extend(run_size(count, args.warmups, args.samples, args.target_ms))

    columns = ("case", "parameters", "payload_bytes", "median_ns", "iqr_ns", "status")
    print("\t".join(columns))
    for row in rows:
        print(
            f"{row['case']}\t{row['parameters']}\t{row['payload_bytes']}\t"
            f"{row['median_ns']:.0f}\t{row['iqr_ns']:.0f}\t{row['status']}"
        )

    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
        print(f"CSV: {args.output}")


if __name__ == "__main__":
    main()
