import pytest
import numpy as np

from scinumtools3.core import DataType
from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, Cursor, PybindException

def test_value_bool():

    # scalar value
    node = ValueNode("foo.bar", True)
    assert node.value == True

    # list value
    node = ValueNode("foo.bar", [True, False, True])
    assert node.value == [True, False, True]

    # numpy value
    node = ValueNode("foo.bar", np.array([[True, False, True], [False, True, False]]))
    val = node.to_numpy()
    assert val.dtype  == np.bool_
    np.testing.assert_array_equal(val, np.array([[True, False, True], [False, True, False]]))
    assert node.shape == [2,3]
    assert node.dtype == DataType.Boolean

    # test exception if units are used
    with pytest.raises(PybindException) as exc_info:
        ValueNode("foo.bar", True, "m")
    exc = exc_info.value    
    assert exc.message == "Units not supported for boolean and string values"
    assert exc.details == "Boolean and string values cannot have an associated quantity or unit."
    assert exc.suggestion == "Remove the units argument when providing a boolean and string values."
            
def test_value_integer():

    # scalar value
    node = ValueNode("foo.bar", 3)
    assert node.value == 3

    # list value
    node = ValueNode("foo.bar", [3, 4, 5])
    assert node.value == [3, 4, 5]

    # numpy value
    node = ValueNode("foo.bar", np.array([[1, 2, 3], [4, 5, 6]]))
    val = node.to_numpy()
    assert val.dtype  == np.int_
    np.testing.assert_array_equal(val, np.array([[1, 2, 3], [4, 5, 6]]))
    assert node.shape == [2,3]
    assert node.dtype == DataType.Integer64
    
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

    # numpy value
    node = ValueNode("foo.bar", np.array([[1.2, 2.3, 3.4], [4.5, 5.6, 6.7]]))
    val = node.to_numpy()
    assert val.dtype  == np.float64
    np.testing.assert_array_equal(val, np.array([[1.2, 2.3, 3.4], [4.5, 5.6, 6.7]]))
    assert node.shape == [2,3]
    assert node.dtype == DataType.Float64
    
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
    
    # numpy value
    node = ValueNode("foo.bar", np.array([["a", "b", "c"], ["d", "e", "fg"]]))
    val = node.to_numpy()
    assert val.dtype.kind == "U"
    np.testing.assert_array_equal(val, np.array([["a", "b", "c"], ["d", "e", "fg"]]))
    assert node.shape == [2,3]
    assert node.dtype == DataType.String
    
    # test exception if units are used
    with pytest.raises(PybindException) as exc_info:
        ValueNode("foo.bar", "baz", "m")
    exc = exc_info.value    
    assert exc.message == "Units not supported for boolean and string values"
    assert exc.details == "Boolean and string values cannot have an associated quantity or unit."
    assert exc.suggestion == "Remove the units argument when providing a boolean and string values."
