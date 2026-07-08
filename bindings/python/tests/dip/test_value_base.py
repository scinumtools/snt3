import pytest
import numpy as np

from scinumtools3.dip import DIP, Environment, ValueNode, Cursor

def test_value_bool():

    # return as 
    dip = DIP()
    dip.add_string((
        "snap bool = none\n"
        "pop bool = false\n"
        "crackle bool[2,3] = [[true,false,true],[false,true,false]]\n"
    ))
    env = dip.parse()

    val = env.request_value("?snap")
    assert val is None

    val = env.request_value("?pop")
    assert isinstance(val, bool)
    assert val == False

    val = env.request_value("?crackle")
    assert isinstance(val, list)
    assert isinstance(val[0][0], bool)
    assert val == [[True,False,True],[False,True,False]]
    
    val = env.request_value("?snap", as_numpy=True)
    assert val is None

    val = env.request_value("?pop", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert val.dtype == np.bool_
    np.testing.assert_array_equal(val, np.array(False))

    val = env.request_value("?crackle", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert val.dtype == np.bool_
    assert val.shape == (2,3)
    np.testing.assert_array_equal(val, np.array([[True,False,True],[False,True,False]]))

def test_value_integer():

    # return as 
    dip = DIP()
    dip.add_string((
        "snap int = none\n"
        "pop int = 3\n"
        "crackle int[2,3] = [[1,2,3],[4,5,6]]\n"
    ))
    env = dip.parse()

    val = env.request_value("?snap")
    assert val is None

    val = env.request_value("?pop")
    assert isinstance(val, int)
    assert val == 3

    val = env.request_value("?crackle")
    assert isinstance(val, list)
    assert isinstance(val[0][0], int)
    assert val == [[1,2,3],[4,5,6]]
    
    val = env.request_value("?snap", as_numpy=True)
    assert val is None

    val = env.request_value("?pop", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert val.dtype == np.int_
    np.testing.assert_array_equal(val, np.array(3))

    val = env.request_value("?crackle", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert val.dtype == np.int_
    assert val.shape == (2,3)
    np.testing.assert_array_equal(val, np.array([[1,2,3],[4,5,6]]))
    
def test_value_float():

    # return as 
    dip = DIP()
    dip.add_string((
        "snap float = none\n"
        "pop float = 3.3\n"
        "crackle float[2,3] = [[1.1,2.2,3.3],[4.4,5.5,6.6]]\n"
    ))
    env = dip.parse()

    val = env.request_value("?snap")
    assert val is None

    val = env.request_value("?pop")
    assert isinstance(val, float)
    assert val == 3.3

    val = env.request_value("?crackle")
    assert isinstance(val, list)
    assert isinstance(val[0][0], float)
    assert val == [[1.1,2.2,3.3],[4.4,5.5,6.6]]
    
    val = env.request_value("?snap", as_numpy=True)
    assert val is None

    val = env.request_value("?pop", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert val.dtype == np.float64
    np.testing.assert_array_equal(val, np.array(3.3))

    val = env.request_value("?crackle", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert val.dtype == np.float64
    assert val.shape == (2,3)
    np.testing.assert_array_equal(val, np.array([[1.1,2.2,3.3],[4.4,5.5,6.6]]))
    
def test_value_string():

    # return as 
    dip = DIP()
    dip.add_string((
        "snap str = none\n"
        "pop str = \"a\"\n"
        "crackle str[2,3] = [[\"a\",\"b\",\"c\"],[\"d\",\"e\",\"f\"]]\n"
    ))
    env = dip.parse()

    val = env.request_value("?snap")
    assert val is None

    val = env.request_value("?pop")
    assert isinstance(val, str)
    assert val == "a"

    val = env.request_value("?crackle")
    assert isinstance(val, list)
    assert isinstance(val[0][0], str)
    assert val == [["a","b","c"],["d","e","f"]]
    
    val = env.request_value("?snap", as_numpy=True)
    assert val is None

    val = env.request_value("?pop", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert np.issubdtype(val.dtype, np.str_)
    np.testing.assert_array_equal(val, np.array("a"))

    val = env.request_value("?crackle", as_numpy=True)
    assert isinstance(val, np.ndarray)
    assert np.issubdtype(val.dtype, np.str_)
    assert val.shape == (2,3)
    np.testing.assert_array_equal(val, np.array([["a","b","c"],["d","e","f"]]))
    
