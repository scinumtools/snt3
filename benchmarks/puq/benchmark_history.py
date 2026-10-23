#!/usr/bin/env python3
"""Run PUQ benchmarks and keep a bounded local history for comparisons."""

from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import platform
import statistics
import subprocess
import sys
import tempfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_HISTORY = REPO_ROOT / "benchmarks" / "puq" / "results" / "history.json"
PYTHON_BENCHMARK = Path(__file__).with_name("python_array_benchmark.py")
DEFAULT_CPP_BENCHMARKS = (
    "benchmark-puq",
    "benchmark-puq-scalar",
    "benchmark-puq-array",
)


def git_metadata() -> dict:
    def git(*args: str) -> str:
        return subprocess.check_output(
            ["git", "-C", str(REPO_ROOT), *args], text=True, stderr=subprocess.DEVNULL
        ).strip()

    try:
        commit = git("rev-parse", "--short=10", "HEAD")
        dirty = bool(git("status", "--porcelain", "--untracked-files=no"))
    except (OSError, subprocess.CalledProcessError):
        commit, dirty = "unknown", None
    return {"commit_short": commit, "working_tree_dirty": dirty}


def find_cpp_benchmarks(explicit: list[Path] | None) -> list[Path]:
    if explicit:
        paths = explicit
    else:
        executable_suffix = ".exe" if os.name == "nt" else ""
        paths = []
        for name in DEFAULT_CPP_BENCHMARKS:
            candidates = [REPO_ROOT / "build-bench" / "bin" / (name + executable_suffix)]
            candidates.extend(
                REPO_ROOT / "build-bench" / "bin" / config / (name + executable_suffix)
                for config in ("Release", "RelWithDebInfo", "Debug")
            )
            found = next((candidate for candidate in candidates if candidate.is_file()), None)
            if found:
                paths.append(found)
    missing = [str(path) for path in paths if not path.is_file()]
    if missing:
        raise RuntimeError("C++ benchmark executable not found: " + ", ".join(missing))
    return paths


def run_cpp(path: Path, repetitions: int, min_time: str) -> dict:
    with tempfile.TemporaryDirectory(prefix="snt-puq-bench-") as directory:
        output_path = Path(directory) / "cpp.json"
        command = [
            str(path.resolve()),
            f"--benchmark_repetitions={repetitions}",
            f"--benchmark_min_time={min_time}",
            "--benchmark_color=true",
            f"--benchmark_out={output_path}",
            "--benchmark_out_format=json",
        ]
        completed = subprocess.run(command)
        if completed.returncode:
            raise RuntimeError(f"{path} failed with exit code {completed.returncode}")
        return json.loads(output_path.read_text(encoding="utf-8"))


def run_python(args: argparse.Namespace) -> dict:
    with tempfile.TemporaryDirectory(prefix="snt-puq-bench-") as directory:
        output_path = Path(directory) / "python.json"
        command = [
            str(args.python.resolve()),
            str(PYTHON_BENCHMARK),
            "--sizes",
            args.sizes,
            "--samples",
            str(args.python_samples),
            "--warmups",
            str(args.python_warmups),
            "--output",
            str(output_path),
        ]
        env = os.environ.copy()
        if args.python_path:
            existing = env.get("PYTHONPATH", "")
            env["PYTHONPATH"] = str(args.python_path.resolve()) + (os.pathsep + existing if existing else "")
        completed = subprocess.run(command, capture_output=True, text=True, env=env)
        if completed.returncode:
            raise RuntimeError(
                f"Python benchmark failed with exit code {completed.returncode}:\n"
                f"{completed.stdout}\n{completed.stderr}"
            )
        return json.loads(output_path.read_text(encoding="utf-8"))


def load_history(path: Path) -> dict:
    if not path.is_file():
        return {"schema_version": 1, "runs": []}
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1 or not isinstance(data.get("runs"), list):
        raise RuntimeError(f"Unsupported benchmark history format: {path}")
    return data


def write_history(path: Path, history: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, delete=False, prefix="history-", suffix=".tmp"
    ) as stream:
        json.dump(history, stream, indent=2)
        stream.write("\n")
        temporary = Path(stream.name)
    temporary.replace(path)


def record(args: argparse.Namespace) -> None:
    cpp_paths = [] if args.skip_cpp else find_cpp_benchmarks(args.cpp_benchmark)
    if not cpp_paths and args.skip_python:
        raise RuntimeError("At least one benchmark suite must be enabled")
    cpp_results = {
        path.stem: run_cpp(path, args.cpp_repetitions, args.cpp_min_time) for path in cpp_paths
    }
    python_result = None if args.skip_python else run_python(args)
    now = dt.datetime.now(dt.timezone.utc)
    git = git_metadata()
    run = {
        "date": now.date().isoformat(),
        "timestamp_utc": now.isoformat(timespec="seconds"),
        "run_id": f"{now.strftime('%Y%m%dT%H%M%SZ')}-{git['commit_short']}",
        **git,
        "host": platform.node(),
        "platform": platform.platform(),
        "cpp": cpp_results,
        "python": python_result,
    }
    history = load_history(args.history)
    history["runs"].append(run)
    history["runs"] = history["runs"][-args.keep :]
    write_history(args.history, history)
    print(f"Recorded {run['run_id']} ({len(history['runs'])}/{args.keep} retained)")
    print(f"History: {args.history}")
    metrics = run_metrics(run)
    if metrics:
        print(f"{'case':<58} {'median':>13}")
        for name, (value, unit) in sorted(metrics.items()):
            print(f"{name:<58} {format_time(time_ns(value, unit)):>13}")


def run_metrics(run: dict) -> dict[str, tuple[float, str]]:
    metrics: dict[str, tuple[float, str]] = {}
    for executable, result in run.get("cpp", {}).items():
        grouped: dict[str, list[float]] = {}
        units: dict[str, str] = {}
        for item in result.get("benchmarks", []):
            if item.get("run_type") != "iteration":
                continue
            name = item.get("run_name", item["name"])
            grouped.setdefault(name, []).append(float(item["real_time"]))
            units[name] = item.get("time_unit", "ns")
        for name, values in grouped.items():
            metrics[f"C++/{executable}/{name}"] = (statistics.median(values), units[name])

    python_result = run.get("python")
    if python_result:
        for size, cases in python_result.get("sizes", {}).items():
            for name, result in cases.items():
                metrics[f"Python/{size}/{name}"] = (
                    float(result["median_ns_per_operation"]),
                    "ns",
                )
    return metrics


def time_ns(value: float, unit: str) -> float:
    return value * {"ns": 1.0, "us": 1_000.0, "ms": 1_000_000.0, "s": 1_000_000_000.0}[unit]


def format_time(ns: float) -> str:
    if ns >= 1_000_000_000:
        return f"{ns / 1_000_000_000:.3f} s"
    if ns >= 1_000_000:
        return f"{ns / 1_000_000:.3f} ms"
    if ns >= 1_000:
        return f"{ns / 1_000:.3f} us"
    return f"{ns:.1f} ns"


def compare(args: argparse.Namespace) -> None:
    runs = load_history(args.history)["runs"]
    if len(runs) < 2:
        raise RuntimeError("At least two retained benchmark runs are needed for comparison")
    # Indices are one-based, with 1 referring to the newest retained run.
    if args.baseline < 1 or args.baseline > len(runs) or args.target < 1 or args.target > len(runs):
        raise RuntimeError(f"Run indices must be between 1 and {len(runs)} (1 is newest)")
    baseline, target = runs[-args.baseline], runs[-args.target]
    print(
        f"Baseline: {baseline.get('date', baseline['timestamp_utc'][:10])} "
        f"{baseline['run_id']} commit {baseline['commit_short']}"
    )
    print(
        f"Target:   {target.get('date', target['timestamp_utc'][:10])} "
        f"{target['run_id']} commit {target['commit_short']}"
    )
    before, after = run_metrics(baseline), run_metrics(target)
    shared = sorted(before.keys() & after.keys())
    if not shared:
        print("No matching benchmark cases between these runs.")
        return
    print(f"{'case':<58} {'baseline':>13} {'target':>13} {'change':>10}")
    for name in shared:
        old, old_unit = before[name]
        new, new_unit = after[name]
        old_ns, new_ns = time_ns(old, old_unit), time_ns(new, new_unit)
        change = ((new_ns / old_ns) - 1.0) * 100.0 if old_ns else float("nan")
        print(f"{name:<58} {format_time(old_ns):>13} {format_time(new_ns):>13} {change:+9.1f}%")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    record_parser = subparsers.add_parser("record", help="Run benchmarks and append to local history")
    record_parser.add_argument("--history", type=Path, default=DEFAULT_HISTORY)
    record_parser.add_argument("--keep", type=int, default=20, help="Maximum retained runs (default: 20)")
    record_parser.add_argument(
        "--cpp-benchmark", type=Path, action="append", help="C++ benchmark executable; repeat to run several"
    )
    record_parser.add_argument("--cpp-repetitions", type=int, default=7)
    record_parser.add_argument("--cpp-min-time", default="0.1s")
    record_parser.add_argument("--skip-cpp", action="store_true", help="Record Python results only")
    record_parser.add_argument("--skip-python", action="store_true", help="Record C++ results only")
    record_parser.add_argument("--python", type=Path, default=Path(sys.executable))
    record_parser.add_argument("--python-path", type=Path, help="Build Python package directory to prepend to PYTHONPATH")
    record_parser.add_argument("--sizes", default="1000,10000,100000")
    record_parser.add_argument("--python-samples", type=int, default=7)
    record_parser.add_argument("--python-warmups", type=int, default=3)
    record_parser.set_defaults(func=record)

    compare_parser = subparsers.add_parser("compare", help="Compare retained runs; index 1 is newest")
    compare_parser.add_argument("--history", type=Path, default=DEFAULT_HISTORY)
    compare_parser.add_argument("--baseline", type=int, default=2)
    compare_parser.add_argument("--target", type=int, default=1)
    compare_parser.set_defaults(func=compare)

    args = parser.parse_args()
    if args.command == "record" and args.keep < 1:
        parser.error("--keep must be at least 1")
    args.func(args)


if __name__ == "__main__":
    main()
