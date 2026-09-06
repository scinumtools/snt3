import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, ValueNodeData, Cursor, PathKind

def test_function_value():

    def snap(env: Environment):
        return ValueNodeData(1.234e5, "mm")

    def crackle(env: Environment):
        return ValueNodeData("pop")
    
    # prepare node collections
    dip = DIP()
    dip.add_function_value("snap", snap)
    dip.add_function_value("crackle", crackle)
    dip.add_string(
        "foo int = 3\n"
        "bar float = snap() m\n"
        "baz str = crackle()"
    )
    env = dip.parse()
    assert env.nodes.size() == 3
    
    assert env.nodes[1].to_string() == "123.4 m"
    assert env['bar'].value == 123.4
    assert env['bar'].units == Quantity("m")

    assert env.nodes[2].to_string() == "\"pop\""
    assert env['baz'].value == "pop"
    
def test_function_nodes():

    def snap(env: Environment):
        node = ValueNode("crackle", "pop")
        return [node]
    
    # prepare node collections
    dip = DIP()
    dip.add_function_nodes("snap", snap) 
    dip.add_string(
        "foo int = 3\n"
        "bar snap()"
    )
    env = dip.parse()

    assert env.nodes.size() == 2
    assert env.nodes[1].to_string() == "\"pop\""
    assert env['bar.crackle'].value == "pop"
