import pytest
import numpy as np
import os

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, OutputFormat, ValueNode

def test_print():
    dip = DIP()
    dip.add_string("foo bool = false")
    dip.add_string("bar int = 1")
    env = dip.parse()

    assert str(dip) == "DIP"
    assert repr(dip) == "DIP"
    assert dip.to_string() == "DIP"

    assert str(env) == "Environment"
    assert repr(env) == "Environment"
    assert env.to_string() == "Environment"

def test_environment_io_placeholders(tmp_path):
    env = Environment()

    with pytest.raises(RuntimeError, match="Loading an environment from HDF5 is not implemented yet"):
        env.load(tmp_path / "environment.h5")

    with pytest.raises(RuntimeError, match="Saving an environment to HDF5 is not implemented yet"):
        env.save(tmp_path / "environment.h5")

    with pytest.raises(RuntimeError, match="Generating a static parameter list is not implemented yet"):
        env.generate(OutputFormat.JSON, tmp_path / "parameters.json")

def test_context_manager():

    with DIP() as dip:
        dip.add_string("foo int = 3")
        env = dip.parse()

    assert env.size == 1
    assert env.nodes[0].name == "foo"
    
def test_add_string():

    dip = DIP()
    dip.add_string("foo bool = false")
    dip.add_string("bar int = 1")
    env = dip.parse()

    assert env.size == 2
    assert env.nodes[0].name == "foo"
    assert env.nodes[1].name == "bar"

def test_add_file(data_dir):
    
    dip = DIP()
    dip.add_file(str(data_dir / "dip/example.dip"))
    env = dip.parse()
    assert env.size == 2
    assert env.nodes[0].name == "foo"
    assert env.nodes[1].name == "bar"

def test_add_source(data_dir):

    dip = DIP()
    dip.add_source("example", str(data_dir / "dip/example.dip"))
    dip.add_string("baz int = {example?bar}")
    env = dip.parse()
    assert env.size == 1
    assert env.nodes[0].name == "baz"
    assert env.nodes[0].value == 2

def test_add_unit():

    dip = DIP()
    dip.add_unit("length", "23*au")
    dip.add_string("baz int = 3 length")
    env = dip.parse()
    assert env.size == 1
    assert env.nodes[0].name == "baz"
    assert env.nodes[0].value == 3
    
