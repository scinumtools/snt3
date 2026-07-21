import pytest
import os

from scinumtools3.dip import DIP, Environment

def test_quick_example():

    full_path = os.getenv("PROJECT_SOURCE_ROOT") + "/examples/dipl/parameters.dip"
    
    with DIP() as dip:
        dip.add_file(full_path)
        env = dip.parse()

    assert env["simulation.fluid.density"].value == 998.2
    assert env["simulation.time.steps"].value == 10000
