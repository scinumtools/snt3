import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, ValueNodeData, Cursor, PathKind

def test_function_value():

    def snap(env: Environment):
        #vd = ValueNodeData(1.23e4, "cm")
        vd = 1.23e4
        return vd
    
    # prepare node collections
    dip = DIP()
    dip.add_function_value("snap", snap) 
    dip.add_string(
        "foo int = 3\n"
        "bar float = snap()"
    )
    env = dip.parse()

    assert env.nodes.size() == 2
    #assert env.nodes[1].to_string() == "1.23e4*cm"
    assert env['bar'].value == 1.23e4

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
