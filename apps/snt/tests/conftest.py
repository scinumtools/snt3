"""Shared fixtures for executable-level CLI tests."""

import os
import json
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
    return normalize_newlines(result.stdout.decode(errors="replace"))


def normalize_newlines(text):
    return text.replace(b"\r\n", b"\n") if isinstance(text, bytes) else text.replace("\r\n", "\n")


def normalize_path(path):
    return str(path).replace("\\", "/")


def fixture_path(path):
    """Escape a native path for substitution inside a JSON test fixture."""
    return json.dumps(str(path))[1:-1]


@pytest.fixture(scope="session")
def source_root():
    return Path(__file__).resolve().parents[3]
