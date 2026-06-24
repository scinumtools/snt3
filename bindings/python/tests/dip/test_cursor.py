import pytest
import numpy as np

from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment, ValueNode, Cursor

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
        params_parsed.append(item.get_path());
        group = item.get_group("bar");
        if key == "crackle":
            clist = item.get_list("pop");
            for item in clist:
                params_parsed.append(item.get_path());
                group = item.get_group("foo");
                params_parsed.append(group.to_string());
        elif key == "lock":
            group = item.get_group("bar");
            params_parsed.append(group.to_string());

    # set a reference list
    params_ref = [
        "jerk.baz = false",
        "jerk.snap[crackle]",
        "jerk.snap[crackle].pop[0]",
        "jerk.snap[crackle].pop[0].foo = true",
        "jerk.snap[crackle].pop[1]",
        "jerk.snap[crackle].pop[1].foo = 3",
        "jerk.snap[crackle].pop[2]",
        "jerk.snap[crackle].pop[2].foo = 4e5",
        "jerk.snap[lock]",
        "jerk.snap[lock].bar = \"shot\"",
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
    vscalar = group.to_scalar()
    assert vscalar == False
    vlist = group.to_list()
    assert len(vlist) == 1
    assert vlist == [False]
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    assert vnumpy == np.array([False])
    
    group = cursor.get_group("jerk.snap[crackle].pop[0].foo")
    assert group.get_shape() == [3]
    vscalar = group.to_scalar()
    assert vscalar == True
    vlist = group.to_list()
    assert len(vlist) == 3
    assert vlist == [True,False,True]
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    assert np.all(vnumpy == np.array([True,False,True]))
    
    group = cursor.get_group("jerk.snap[crackle].pop[1].foo")
    vscalar = group.to_scalar()
    assert vscalar == 1
    vlist = group.to_list()
    assert len(vlist) == 6
    assert vlist == [1,2,3,4,5,6]
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    assert np.all(vnumpy == np.array([[1,2,3],[4,5,6]]))
    assert vnumpy.shape == (2,3)
    
    group = cursor.get_group("jerk.snap[crackle].pop[2].foo")
    vscalar = group.to_scalar()
    assert vscalar == 4e5
    vlist = group.to_list()
    assert len(vlist) == 1
    assert vlist == [4e5]
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    assert vnumpy == np.array([4e5])

    group = cursor.get_group("jerk.snap[lock].bar")
    vscalar = group.to_scalar()
    assert vscalar == "shot"
    vlist = group.to_list()
    assert len(vlist) == 1
    assert vlist == ["shot"]
    vnumpy = group.to_numpy()
    assert type(vnumpy) == np.ndarray
    assert vnumpy == np.array(["shot"])
    
