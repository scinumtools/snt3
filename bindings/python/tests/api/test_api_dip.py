import pytest

from scinumtools3.api.dip import DIPParse

def test_parse():

    c = DIPParse()

    c.argument_add("string",["foo int = 32 cm"])
    c.argument_add("string",["bar float = 13.3e4 kg"])
    c.argument_add("string",["baz str = \"snap\""])
    c.argument_print()
    
    # TODO: test all arguments

    assert c.execute() == (
        "foo = 32 cm\n"
        "bar = 1.33e5 kg\n"
        "baz = \"snap\"\n"
    )
