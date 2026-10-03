from pathlib import Path

import pytest

from scinumtools3.dip import (
    Adapter,
    DIP,
    ExistingOutputPolicy,
    run_adapter,
    run_adapter_project,
    run_adapter_snapshot,
)


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


def test_replace_registered_regenerates_and_preserves_unrelated_files(tmp_path: Path):
    (tmp_path / "parameters.dip").write_text("run\n  steps int = 2\n")
    (tmp_path / "DIPfile").write_text('code[]\n  file = "parameters.dip"\n')
    output = tmp_path / "output"
    adapter = ExampleAdapter()
    policy = ExistingOutputPolicy.ReplaceRegistered

    run_adapter_project(tmp_path / "DIPfile", adapter, output, "run.diph5")
    (output / "unrelated.txt").write_text("keep me")
    (output / "control/settings.ini").write_text("old settings")
    with pytest.raises(RuntimeError):
        run_adapter_project(tmp_path / "DIPfile", adapter, output, "run.diph5")

    (tmp_path / "parameters.dip").write_text("run\n  steps int = 4\n")
    written = run_adapter_project(
        tmp_path / "DIPfile", adapter, output, "run.diph5", existing_output_policy=policy
    )
    assert len(written) == 4
    assert (output / "control/settings.ini").read_text() == "steps=4\n"
    assert (output / "data/steps.csv").read_text() == "0\n1\n2\n3\n"
    assert (output / "unrelated.txt").read_text() == "keep me"
    assert len(run_adapter_snapshot(
        output / "run.diph5", adapter, output, existing_output_policy=policy
    )) == 3
    assert (output / "unrelated.txt").read_text() == "keep me"


def test_replace_registered_rejects_conflicts(tmp_path: Path):
    parser = DIP()
    parser.add_string("value int = 1\n")
    env = parser.parse()
    output = tmp_path / "output"
    output.mkdir()
    (output / "directory").mkdir()
    (output / "parent").write_text("original")

    class OneFile(Adapter):
        def __init__(self, path):
            super().__init__()
            self.path = path

        def plan(self, env, context):
            context.add_text(self.path, "new")

    for path in ("directory", "parent/child"):
        with pytest.raises(RuntimeError):
            run_adapter(env, OneFile(path), output,
                        existing_output_policy=ExistingOutputPolicy.ReplaceRegistered)
    assert (output / "parent").read_text() == "original"

    link = output / "link"
    try:
        link.symlink_to(output / "parent")
    except (OSError, NotImplementedError):
        return
    for path in ("link", "link/child"):
        with pytest.raises(RuntimeError):
            run_adapter(env, OneFile(path), output,
                        existing_output_policy=ExistingOutputPolicy.ReplaceRegistered)
    assert link.is_symlink()


def test_failed_python_stream_preserves_registered_files(tmp_path: Path):
    parser = DIP()
    parser.add_string("value int = 1\n")
    env = parser.parse()
    output = tmp_path / "output"
    output.mkdir()
    (output / "first.txt").write_text("old first")
    (output / "unrelated.txt").write_text("unrelated")

    def fail(write):
        write(b"partial")
        raise ValueError("stream failed")

    class FailingAdapter(Adapter):
        def plan(self, env, context):
            context.add_text("first.txt", "new first")
            context.add_stream("second.txt", fail)

    with pytest.raises(ValueError, match="stream failed"):
        run_adapter(env, FailingAdapter(), output,
                    existing_output_policy=ExistingOutputPolicy.ReplaceRegistered)
    assert (output / "first.txt").read_text() == "old first"
    assert (output / "unrelated.txt").read_text() == "unrelated"
    assert not (output / "second.txt").exists()


def test_sync_registered_prunes_previous_outputs_and_keeps_default_reject(tmp_path: Path):
    parser = DIP()
    parser.add_string("value int = 1\n")
    env = parser.parse()
    output = tmp_path / "output"

    class Files(Adapter):
        def __init__(self, names):
            super().__init__()
            self.names = names

        def plan(self, env, context):
            for name in self.names:
                context.add_text(name, name)

    policy = ExistingOutputPolicy.SyncRegistered
    assert len(run_adapter(env, Files(["keep.txt", "stale.txt"]), output,
                           existing_output_policy=policy)) == 2
    (output / "unregistered.txt").write_text("keep me")
    with pytest.raises(RuntimeError):
        run_adapter(env, Files(["keep.txt"]), output)
    assert len(run_adapter(env, Files(["keep.txt"]), output,
                           existing_output_policy=policy)) == 1
    assert not (output / "stale.txt").exists()
    assert (output / "unregistered.txt").read_text() == "keep me"
    assert (output / ".snt-adapter-manifest").is_file()
