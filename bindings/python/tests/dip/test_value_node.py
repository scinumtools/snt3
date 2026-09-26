import gc
import weakref

import pytest
import numpy as np

from scinumtools3.core import DataType
from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, Cursor, PybindException

def test_tags_and_metadata_defaults():
    # Newly created nodes have empty tags and metadata.
    node = ValueNode("foo", 3)
    assert node.tags == []
    assert node.metadata.description == ""


def test_query_tags_and_metadata():
    dip = DIP()
    dip.add_string(
        'foo int = 3\n'
        '  !tags ["export", "science"]\n'
        '  ?descr "Exported parameter"\n'
        '  ?native ["Foo", "FOO"]\n'
        'bar int = 4\n'
    )
    env = dip.parse()
    # Query results expose tags and metadata directly on each node.
    nodes = env.request_group("?", ["export"])
    assert len(nodes) == 1
    node = nodes[0]
    assert node.name == "foo"
    assert node.tags == ["export", "science"]
    assert isinstance(node.tags, list)
    assert isinstance(node.metadata, type(env["foo"].metadata))
    assert node.metadata.description == "Exported parameter"
    assert node.metadata.native == ["Foo", "FOO"]

    # Editing the returned list changes only the local copy.
    tags = node.tags
    tags.clear()
    tags.append("local")
    assert tags == ["local"]
    assert node.tags == ["export", "science"]
    # Environment queries still use the original tags.
    assert len(env.request_group("?", ["export"])) == 1
    with pytest.raises(RuntimeError, match="Node request returns empty node group"):
        env.request_group("?", ["local"])

    # Properties and metadata fields cannot be reassigned.
    with pytest.raises(AttributeError):
        node.tags = []
    with pytest.raises(AttributeError):
        node.metadata = node.metadata
    with pytest.raises(AttributeError):
        node.metadata.description = "Changed"

    # Holding metadata keeps its node alive after other owners are deleted.
    metadata = node.metadata
    node_ref = weakref.ref(node)
    del node, nodes, env, dip
    gc.collect()
    assert node_ref() is not None
    assert metadata.description == "Exported parameter"
    assert metadata.native == ["Foo", "FOO"]
    # Releasing metadata also releases the node.
    del metadata
    gc.collect()
    assert node_ref() is None


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
    assert exc.message == "Units not supported for value type"
    assert exc.details == "A value of data type `bool` cannot have units. Units are only supported for integer and floating-point values."
    assert exc.suggestion == "Remove the units or provide a numeric value."
            
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
    assert exc.message == "Units not supported for value type"
    assert exc.details == "A value of data type `str` cannot have units. Units are only supported for integer and floating-point values."
    assert exc.suggestion == "Remove the units or provide a numeric value."
