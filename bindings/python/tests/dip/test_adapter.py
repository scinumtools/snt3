from pathlib import Path

import pytest

from scinumtools3.dip import Adapter, DIP, run_adapter, run_adapter_project, run_adapter_snapshot


class ExampleAdapter(Adapter):
    def plan(self, env, context):
        steps = env["run.steps"].value
        context.add_text("control/settings.ini", f"steps={steps}\n")
        context.add_binary("control/marker.bin", b"\x00\xff")
        context.add_stream("data/steps.csv", lambda write: [
            write(f"{index}\n".encode()) for index in range(steps)
        ])


def test_multiple_outputs_from_project_and_snapshot(tmp_path: Path):
    (tmp_path / "parameters.dip").write_text("run\n  steps int = 3\n")
    (tmp_path / "DIPfile").write_text('code[]\n  file = "parameters.dip"\n')
    adapter = ExampleAdapter()

    project = tmp_path / "project"
    written = run_adapter_project(tmp_path / "DIPfile", adapter, project, "run.diph5")
    assert len(written) == 4
    assert (project / "control/settings.ini").read_text() == "steps=3\n"
    assert (project / "control/marker.bin").read_bytes() == b"\x00\xff"
    assert (project / "data/steps.csv").read_text() == "0\n1\n2\n"

    loaded = tmp_path / "loaded"
    assert len(run_adapter_snapshot(project / "run.diph5", adapter, loaded)) == 3
    for name in ("control/settings.ini", "control/marker.bin", "data/steps.csv"):
        assert (project / name).read_bytes() == (loaded / name).read_bytes()

    parser = DIP()
    parser.add_project(tmp_path / "DIPfile")
    env = parser.parse()
    assert len(run_adapter(env, adapter, tmp_path / "parsed")) == 3


def test_path_conflict_prevents_python_stream_callback(tmp_path: Path):
    parser = DIP()
    parser.add_string("run\n  steps int = 1\n")
    env = parser.parse()
    called = []

    class ConflictAdapter(Adapter):
        def plan(self, env, context):
            context.add_text("same.txt", "first")
            context.add_stream("same.txt", lambda write: called.append(True))

    with pytest.raises(RuntimeError, match="conflicts"):
        run_adapter(env, ConflictAdapter(), tmp_path / "invalid")
    assert called == []
    assert not (tmp_path / "invalid").exists()
