import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode

def test_select_node():

    # initialize boolean nodes
    dip = DIP()
    dip.add_string("foo bool = false");
    dip.add_string("bar bool[2,3] = [[true,false,true],[false,true,false]]");
    env = dip.parse();

    assert env.size == 2
    assert type(env.nodes[0]) == ValueNode
    assert env.nodes[0].name == "foo"
    assert env.nodes[0].value == False
    np.testing.assert_array_equal(env.nodes[1].value, np.array([[True,False,True],[False,True,False]]))

    # initialize integer nodes
    dip = DIP()
    dip.add_string("foo int = 3 cm");
    dip.add_string("bar int[2,3] = [[1,2,3],[4,5,6]]");
    env = dip.parse();

    assert env.size == 2
    assert env.nodes[0].name == "foo"
    assert env.nodes[0].value == 3
    assert env.nodes[0].units == Quantity('cm')
    np.testing.assert_array_equal(env.nodes[1].value, np.array([[1,2,3],[4,5,6]]))

    # initialize floating point nodes
    dip = DIP()
    dip.add_string("foo float = 3.45");
    dip.add_string("bar float[2,3] = [[1.2,2.3,3.4],[4.5,5.6,6.7]]");
    env = dip.parse();

    assert env.size == 2
    assert env.nodes[0].name == "foo"
    assert env.nodes[0].value == 3.45
    np.testing.assert_array_equal(env.nodes[1].value, np.array([[1.2,2.3,3.4],[4.5,5.6,6.7]]))

    # initialize string nodes
    dip = DIP()
    dip.add_string("foo str = 'A'");
    dip.add_string("bar str[2,3] = [['a','b','c'],['d','e','f']]");
    env = dip.parse();

    assert env.size == 2
    assert env.nodes[0].name == "foo"
    assert env.nodes[0].value == 'A'
    np.testing.assert_array_equal(env.nodes[1].value, np.array([['a','b','c'],['d','e','f']]))
    
def test_request_nodes():

    # request a single node
    dip = DIP()
    dip.add_string("foo bool = false");
    dip.add_string("bar bool[2,3] = [[true,false,true],[false,true,false]]");
    env = dip.parse();

    nodes = env.request("bar");
    assert len(nodes) == 1
    assert nodes[0].name == "bar"
    np.testing.assert_array_equal(nodes[0].value, np.array([[True,False,True],[False,True,False]]))

    # request a node group
    dip = DIP()
    dip.add_string("foo.bar bool = false");
    dip.add_string("foo.baz int = 3");
    env = dip.parse();

    nodes = env.request("foo");
    assert len(nodes) == 2
    assert nodes[0].name == "bar"
    assert nodes[0].value == False
    assert nodes[1].name == "baz"
    assert nodes[1].value == 3
