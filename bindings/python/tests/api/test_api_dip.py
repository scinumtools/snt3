import pytest

from scinumtools3.api.dip import DIPParse

def test_parse():

    c = DIPParse()

    c.argument_input("string",["foo int = 32 cm"])
    c.argument_input("string",["bar float = 13.3e4 kg"])
    c.argument_input("string",["baz str = \"snap\""])
    c.argument_print()
    
    # TODO: test all arguments

    assert c.execute() == (
        "foo = 32 cm\n"
        "bar = 1.33e5 kg\n"
        "baz = \"snap\"\n"
    )
