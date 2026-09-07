import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, ValueNodeData, Cursor, PathKind

def test_function_scalar_value():

    def jerk(env: Environment):
        return ValueNodeData(True)
    def snap(env: Environment):
        return ValueNodeData(1234, "kg")
    def crackle(env: Environment):
        return ValueNodeData(1.234e5, "mm")
    def pop(env: Environment):
        return ValueNodeData("pop")
    
    # prepare node collections
    dip = DIP()
    dip.add_function_value("jerk", jerk)
    dip.add_function_value("snap", snap)
    dip.add_function_value("crackle", crackle)
    dip.add_function_value("pop", pop)
    dip.add_string(
        "jerk bool = jerk()\n"
        "snap int = snap() g\n"
        "crackle float = crackle() m\n"
        "pop str = pop()"
    )
    env = dip.parse()
    assert env.nodes.size() == 4
    
    assert env.nodes[0].to_string() == "true"
    assert env['jerk'].value == True
    
    assert env.nodes[1].to_string() == "1234000 g"
    assert env['snap'].value == 1234000
    assert env['snap'].units == Quantity("g")

    assert env.nodes[2].to_string() == "123.4 m"
    assert env['crackle'].value == 123.4
    assert env['crackle'].units == Quantity("m")

    assert env.nodes[3].to_string() == "\"pop\""
    assert env['pop'].value == "pop"

def test_function_list_value():

    def jerk(env: Environment):
        return ValueNodeData([True,False,True])
    def snap(env: Environment):
        return ValueNodeData([1234,2345,3456], "kg")
    def crackle(env: Environment):
        return ValueNodeData([1.234e5,2.34e5,3.4e5], "mm")
    def pop(env: Environment):
        return ValueNodeData(["pop","op","p"])
    
    # prepare node collections
    dip = DIP()
    dip.add_function_value("jerk", jerk)
    dip.add_function_value("snap", snap)
    dip.add_function_value("crackle", crackle)
    dip.add_function_value("pop", pop)
    dip.add_string(
        "jerk bool[3] = jerk()\n"
        "snap int[3] = snap() g\n"
        "crackle float[3] = crackle() m\n"
        "pop str[3] = pop()"
    )
    env = dip.parse()
    assert env.nodes.size() == 4
    
    assert env.nodes[0].to_string() == "[true, false, true]"
    assert env['jerk'].value == [True, False, True]
    
    assert env.nodes[1].to_string() == "[1234000, 2345000, 3456000] g"
    assert env['snap'].value == [1234000, 2345000, 3456000]
    assert env['snap'].units == Quantity("g")

    assert env.nodes[2].to_string() == "[123.4, 234, 340] m"
    assert env['crackle'].value == [123.4, 234, 340]
    assert env['crackle'].units == Quantity("m")

    assert env.nodes[3].to_string() == '["pop", "op", "p"]'
    assert env['pop'].value == ["pop", "op", "p"]

def test_function_array_value():

    def jerk(env: Environment):
        return ValueNodeData(np.array([[True,False,True],[False,True,False]]))
    def snap(env: Environment):
        return ValueNodeData(np.array([[12,23,34],[45,56,67]]), "kg")
    def crackle(env: Environment):
        return ValueNodeData(np.array([[1.2e3,2.3e4,5.6e7],[6.7e8,7.8e9,8.9e10]]), "mm")
    def pop(env: Environment):
        return ValueNodeData(np.array([["pop","op","p"],["o","po","opo"]]))
    
    # prepare node collections
    dip = DIP()
    dip.add_function_value("jerk", jerk)
    dip.add_function_value("snap", snap)
    dip.add_function_value("crackle", crackle)
    dip.add_function_value("pop", pop)
    dip.add_string(
        "jerk bool[2,3] = jerk()\n"
        "snap int[2,3] = snap() g\n"
        "crackle float[2,3] = crackle() m\n"
        "pop str[2,3] = pop()"
    )
    env = dip.parse()
    assert env.nodes.size() == 4
    
    assert env.nodes[0].to_string() == "[[true, false, true], [false, true, false]]"
    np.testing.assert_array_equal(env['jerk'].to_numpy(), [[True, False, True], [False, True, False]])
    
    assert env.nodes[1].to_string() == "[[12000, 23000, 34000], [45000, 56000, 67000]] g"
    np.testing.assert_array_equal(env['snap'].to_numpy(), [[12000, 23000, 34000], [45000, 56000, 67000]])
    assert env['snap'].units == Quantity("g")

    assert env.nodes[2].to_string() == "[[1.2, 23, 5.6e4], [6.7e5, 7.8e6, 8.9e7]] m"
    np.testing.assert_array_equal(env['crackle'].to_numpy(), [[1.2, 23, 5.6e4], [6.7e5, 7.8e6, 8.9e7]])
    assert env['crackle'].units == Quantity("m")

    assert env.nodes[3].to_string() == '[["pop", "op", "p"], ["o", "po", "opo"]]'
    np.testing.assert_array_equal(env['pop'].to_numpy(), [["pop", "op", "p"], ["o", "po", "opo"]])
    
def test_function_nodes():

    def bar(env: Environment):
        return [
            ValueNode("jerk", False),
            ValueNode("snap", 1234, "m"),
            ValueNode("crackle", 1.23e4, "kg"),
            ValueNode("pop", "pop"),
        ]
    
    # prepare node collections
    dip = DIP()
    dip.add_function_nodes("bar", bar) 
    dip.add_string(
        "foo bar()"
    )
    env = dip.parse()
    assert env.nodes.size() == 4
    
    assert env.nodes[0].to_string() == "false"
    assert env['foo.jerk'].value == False

    assert env.nodes[1].to_string() == "1234 m"
    assert env['foo.snap'].value == 1234
    assert env['foo.snap'].units == Quantity("m")

    assert env.nodes[2].to_string() == "1.23e4 kg"
    assert env['foo.crackle'].value == 1.23e4
    assert env['foo.crackle'].units == Quantity("kg")

    assert env.nodes[3].to_string() == "\"pop\""
    assert env['foo.pop'].value == "pop"
