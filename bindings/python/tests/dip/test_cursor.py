import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, Cursor

def test_cursor_print():
    
    # prepare node collections
    dip = DIP()
    dip.add_string(
        "foo\n"
        "  jerk[]\n"
        "    snap[crackle]\n"
        "      pop int = 3\n"
    )
    env = dip.parse()
    cursor = env.request_cursor()

    # group collection
    cur = cursor.get_group("foo");
    assert repr(cur) == "Cursor('foo')" 
    assert str(cur) == "Cursor('foo')"

    # list collection
    cur = cursor.get_list("foo.jerk");
    assert repr(cur) == "[Cursor('foo.jerk[0]')]"
    assert str(cur) == "[Cursor('foo.jerk[0]')]"

    # map collection
    cur = cursor.get_map("foo.jerk[0].snap");
    assert repr(cur) == "{'crackle': Cursor('foo.jerk[0].snap[crackle]')}"
    assert str(cur) == "{'crackle': Cursor('foo.jerk[0].snap[crackle]')}"

    # group collection with a value
    cur = cursor.get_group("foo.jerk[0].snap[crackle].pop");
    assert repr(cur) == "Cursor('foo.jerk[0].snap[crackle].pop', 3)"
    assert str(cur) == "Cursor('foo.jerk[0].snap[crackle].pop', 3)"
    
def test_cursor_traverse():

    # prepare node collections
    dip = DIP()
    dip.add_string((
        "jerk.snap[crackle]\n"
        "  bar str = \"pot\"\n"
        "  pop[]\n"
        "    foo bool = true\n"
        "  pop[]\n"
        "    foo int = 3\n"
        "  pop[]\n"
        "    foo float = 4e5\n"
        "jerk.snap[lock]\n"
        "  bar str = \"shot\"\n"
        "jerk.baz bool = false\n"
    ))
    env = dip.parse()
    cursor = env.request_cursor()

    # save parsed list names
    params_parsed = []
    group = cursor.get_group("jerk.baz");
    params_parsed.append(group.to_string());
    cmap = cursor.get_map("jerk.snap");
    for key, item in cmap.items():
        params_parsed.append(item.to_string());
        group = item.get_group("bar");
        if key == "crackle":
            clist = item.get_list("pop");
            for item in clist:
                params_parsed.append(item.to_string());
                group = item.get_group("foo");
                params_parsed.append(group.to_string());
        elif key == "lock":
            group = item.get_group("bar");
            params_parsed.append(group.to_string());

    # set a reference list
    params_ref = [
        "Cursor('jerk.baz', false)",
        "Cursor('jerk.snap[crackle]')",
        "Cursor('jerk.snap[crackle].pop[0]')",
        "Cursor('jerk.snap[crackle].pop[0].foo', true)",
        "Cursor('jerk.snap[crackle].pop[1]')",
        "Cursor('jerk.snap[crackle].pop[1].foo', 3)",
        "Cursor('jerk.snap[crackle].pop[2]')",
        "Cursor('jerk.snap[crackle].pop[2].foo', 4e5)",
        "Cursor('jerk.snap[lock]')",
        "Cursor('jerk.snap[lock].bar', \"shot\")",
    ]

    # compare values
    assert sorted(params_parsed) == params_ref

def test_cursor_values():

    # prepare node collections
    dip = DIP()
    dip.add_string((
        "jerk.snap[crackle]\n"
        "  bar str = \"pot\"\n"
        "  pop[]\n"
        "    foo bool[3] = [true,false,true]\n"
        "  pop[]\n"
        "    foo int[2,3] = [[1,2,3],[4,5,6]]\n"
        "  pop[]\n"
        "    foo float = 4e5\n"
        "jerk.snap[lock]\n"
        "  bar str = \"shot\"\n"
        "jerk.baz bool = false\n"
    ))
    env = dip.parse()
    cursor = env.request_cursor()

    group = cursor.get_group("jerk.baz")
    assert type(group.value) == bool
    assert group.value == False
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    np.testing.assert_array_equal(vnumpy, np.array(False))
    
    group = cursor.get_group("jerk.snap[crackle].pop[0].foo")
    assert group.shape == [3]
    assert type(group.value) == list
    assert group.value == [True,False,True]
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    np.testing.assert_array_equal(vnumpy, np.array([True,False,True]))
    
    group = cursor.get_group("jerk.snap[crackle].pop[1].foo")
    assert group.shape == [2,3]
    assert type(group.value) == list
    assert group.value == [[1,2,3],[4,5,6]]
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    np.testing.assert_array_equal(vnumpy, np.array([[1, 2, 3], [4, 5, 6]]))
    assert vnumpy.shape == (2,3)
    
    group = cursor.get_group("jerk.snap[crackle].pop[2].foo")
    assert group.shape == [1]
    assert type(group.value) == float
    assert group.value == 4e5
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    np.testing.assert_array_equal(vnumpy, np.array(4e5))

    group = cursor.get_group("jerk.snap[lock].bar")
    assert group.shape == [1]
    assert type(group.value) == str
    assert group.value == "shot"
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    np.testing.assert_array_equal(vnumpy, np.array(["shot"]))
    
