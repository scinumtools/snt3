import pytest
import tempfile
import os

from scinumtools3.api.dip import DIPParse

def test_add_string():

    c = DIPParse()
    c.argument_add("string",[
        "foo int = 32 cm\n"
        "bar float = 13.3e4 kg\n"
        "baz str = \"snap\""
    ])
    c.argument_print()
    
    assert c.execute() == (
        "foo = 32 cm\n"
        "bar = 1.33e5 kg\n"
        "baz = \"snap\"\n"
    )

def test_add_file():

    with tempfile.NamedTemporaryFile(mode="w", suffix=".dip", delete=False) as f:
        f.write(
            "foo int = 32 cm\n"
            "bar float = 13.3e4 kg\n"
            "baz str = \"snap\""
        )
        filename = f.name
        
    c = DIPParse()
    c.argument_add("file",[filename])
    c.argument_print()
    
    assert c.execute() == (
        "foo = 32 cm\n"
        "bar = 1.33e5 kg\n"
        "baz = \"snap\"\n"
    )

    os.unlink(filename)

def test_request():

    c = DIPParse()
    c.argument_add("string",[
        "foo int = 32 cm\n"
        "bar float = 13.3e4 kg"
    ])
    c.argument_request("bar")
    c.argument_print()
    
    assert c.execute() == (
        "bar = 1.33e5 kg\n"
    )

def test_tags():

    c = DIPParse()
    c.argument_add("string",[
        "foo int = 32 cm\n"
        "bar float = 13.3e4 kg\n"
        "  !tags [\"baz\"]"
    ])
    c.argument_tags(["baz"])
    c.argument_print()
    
    assert c.execute() == (
        "bar = 1.33e5 kg\n"
    )
    
