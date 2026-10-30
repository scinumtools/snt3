"""Shared fixtures for executable-level CLI tests."""

import os
import subprocess
from pathlib import Path

import pytest


@pytest.fixture(scope="session")
def snt_executable():
    return Path(os.environ["SNT_EXECUTABLE"]).resolve()


@pytest.fixture
def cli(snt_executable):
    def run(*args, cwd=None):
        return subprocess.run(
            [str(snt_executable), *(str(arg) for arg in args)],
            cwd=cwd,
            capture_output=True,
        )

    return run


def checked(result, status=0):
    assert result.returncode == status, (
        f"exit {result.returncode}, expected {status}\n"
        f"stdout: {result.stdout.decode(errors='replace')}\n"
        f"stderr: {result.stderr.decode(errors='replace')}"
    )
    return result.stdout.decode(errors="replace")


@pytest.fixture(scope="session")
def source_root():
    return Path(__file__).resolve().parents[3]
