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

    group = cursor.get_group("jerk.baz")
    vector = group.get_vector()
    assert len(vector) == 1
    assert vector[0] == False
    scalar = group.get_scalar()
    assert scalar == False

    group = cursor.get_group("jerk.snap[crackle].pop[1].foo")
    vector = group.get_vector()
    assert len(vector) == 1
    assert vector[0] == 3
    scalar = group.get_scalar()
    assert scalar == 3
    
    group = cursor.get_group("jerk.snap[crackle].pop[2].foo")
    vector = group.get_vector()
    assert len(vector) == 1
    assert vector[0] == 4e5
    scalar = group.get_scalar()
    assert scalar == 4e5
    
    group = cursor.get_group("jerk.snap[lock].bar")
    vector = group.get_vector()
    assert len(vector) == 1
    assert vector[0] == "shot"
    scalar = group.get_scalar()
    assert scalar == "shot"
    
