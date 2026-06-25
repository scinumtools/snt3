import pytest
import importlib.util
import sys
import os
import glob
import subprocess

import scinumtools3

def test_version():

    p = subprocess.Popen("source settings.env;echo $CODE_VERSION", stdout=subprocess.PIPE, shell=True, executable='/bin/bash')
    assert scinumtools3.__version__ == p.stdout.readlines()[0].strip().decode("utf-8")


from pathlib import Path

import scinumtools3


def test_version():
    env = {}

    ROOT = Path(__file__).resolve().parents[4]
    ENV_FILE = ROOT / "settings.env"
    
    for line in Path(ENV_FILE).read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        key, value = line.split("=", 1)
        env[key] = value

    assert scinumtools3.__version__ == env["CODE_VERSION"]
