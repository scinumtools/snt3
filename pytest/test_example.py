import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode

def test_example():

    length = Quantity("1*m")
    length = length.convert("km");
    assert str(length) == '1e-3*km'
    
    dip = DIP()
    dip.add_string("foo int m");
    dip.add_string("foo = 3 km");
    env = dip.parse();
    assert str(env.nodes[0]) == '3000 m'
    assert env.nodes[0].value == 3000
    assert env.nodes[0].units == Quantity('m')
