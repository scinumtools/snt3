import pytest
import numpy as np

from scinumtools3.dip import DIP, Environment, ValueNode

def test_parsing_value():

    # initialize boolean nodes
    dip = DIP()
    dip.add_string("foo bool = false");
    dip.add_string("bar bool[2,3] = [[true,false,true],[false,true,false]]");
    env = dip.parse();

    assert env.size == 2
    assert type(env[0]) == ValueNode
    assert env[0].value == False
    np.testing.assert_array_equal(env[1].value, np.array([[True,False,True],[False,True,False]]))

    # initialize integer nodes
    dip = DIP()
    dip.add_string("foo int = 3");
    dip.add_string("bar int[2,3] = [[1,2,3],[4,5,6]]");
    env = dip.parse();

    assert env.size == 2
    assert env[0].value == 3
    np.testing.assert_array_equal(env[1].value, np.array([[1,2,3],[4,5,6]]))

    # initialize floating point nodes
    dip = DIP()
    dip.add_string("foo float = 3.45");
    dip.add_string("bar float[2,3] = [[1.2,2.3,3.4],[4.5,5.6,6.7]]");
    env = dip.parse();

    assert env.size == 2
    assert env[0].value == 3.45
    np.testing.assert_array_equal(env[1].value, np.array([[1.2,2.3,3.4],[4.5,5.6,6.7]]))

    # initialize string nodes
    dip = DIP()
    dip.add_string("foo str = 'A'");
    dip.add_string("bar str[2,3] = [['a','b','c'],['d','e','f']]");
    env = dip.parse();

    assert env.size == 2
    #assert env[0].value == 'A'
    print(env[1].value)
    np.testing.assert_array_equal(env[1].value, np.array([['a','b','c'],['d','e','f']]))
    
