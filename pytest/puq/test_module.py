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
