import pytest

from scinumtools3.api.dip import DIPParse

def test_parse():

    c = DIPParse()

    c.argument_string("foo int = 32 cm")
    c.argument_print()
    
    # TODO: test all arguments

    assert c.execute() == "foo = 32 cm\n"
