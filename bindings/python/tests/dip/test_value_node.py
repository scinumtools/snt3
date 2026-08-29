import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, Cursor

def test_value_bool():

    # scalar value
    node = ValueNode("foo.bar", True)
    assert node.value == True

    # list value
    node = ValueNode("foo.bar", [True, False, True])
    assert node.value == [True, False, True]
    
def test_value_integer():

    # scalar value
    node = ValueNode("foo.bar", 3)
    assert node.value == 3

    # list value
    node = ValueNode("foo.bar", [3, 4, 5])
    assert node.value == [3, 4, 5]

    # with units
    node = ValueNode("foo.bar", 3, "cm")
    assert node.units == Quantity("cm")
    node = ValueNode("foo.bar", [3, 4, 5], "cm")
    assert node.units == Quantity("cm")
    
def test_value_float():

    # scalar value
    node = ValueNode("foo.bar", 3.4)
    assert node.value == 3.4

    # list value
    node = ValueNode("foo.bar", [3.4, 4.5, 5.6])
    assert node.value == [3.4, 4.5, 5.6]

    # with units
    node = ValueNode("foo.bar", 3.4, "cm")
    assert node.units == Quantity("cm")
    node = ValueNode("foo.bar", [3.4, 4.5, 5.6], "cm")
    assert node.units == Quantity("cm")
    
def test_value_string():

    # scalar value
    node = ValueNode("foo.bar", "baz")
    assert node.value == "baz"

    # list value
    node = ValueNode("foo.bar", ["snap", "crackle", "pop"])
    assert node.value == ["snap", "crackle", "pop"]
    
