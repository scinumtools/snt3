import pytest
import numpy as np
import os

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode

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
    
