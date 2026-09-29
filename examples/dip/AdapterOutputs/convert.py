"""Generate JSON and CSV inputs for a different downstream application."""

import argparse
import json
from pathlib import Path

from scinumtools3.dip import Adapter, run_adapter_project


class AnalysisAdapter(Adapter):
    def plan(self, env, context):
        steps = env["run.steps"].value
        dt = env["run.dt"].value
        context.add_text("analysis/job.json", json.dumps({"steps": steps, "dt_seconds": dt}, indent=2) + "\n")

        def write_rows(write):
            write(b"step,time_seconds\n")
            for index in range(steps):
                write(f"{index},{index * dt}\n".encode())

        context.add_stream("analysis/times.csv", write_rows)


if __name__ == "__main__":
    arguments = argparse.ArgumentParser()
    arguments.add_argument("--project", type=Path, default=Path(__file__).with_name("DIPfile"))
    arguments.add_argument("--output", type=Path, default=Path("build/adapter-python"))
    args = arguments.parse_args()
    for path in run_adapter_project(args.project, AnalysisAdapter(), args.output):
        print(path)
