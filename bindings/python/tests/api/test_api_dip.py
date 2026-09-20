import pytest
import tempfile
import os

from scinumtools3.api.dip import DIPParse
from scinumtools3.dip import DIP, Environment


def test_save(tmp_path):
    file = tmp_path / "parameters.diph5"
    c = DIPParse()
    c.argument_add("string", ["simulation.steps int = 100\nsimulation.enabled bool = true\n"])
    c.argument_request("simulation.steps")
    c.argument_value("integer")
    c.argument_save(str(file))
    assert not file.exists()  # Configuration does not perform I/O.
    assert c.execute() == "100\n"
    env = Environment()
    env.load(file)
    assert env["simulation.steps"].value == 100
    assert env["simulation.enabled"].value is True


def test_generate(tmp_path):
    file = tmp_path / "parameters.json"
    c = DIPParse()
    c.argument_add("string", ["simulation.steps int = 100\n"])
    c.argument_generate("json", str(file))

    assert not file.exists()  # Configuration does not perform I/O.
    assert c.execute() == ""
    assert '"simulation"' in file.read_text()


def test_load(tmp_path):
    file = tmp_path / "parameters.diph5"
    dip = DIP()
    dip.add_string("simulation.steps int = 100\n")
    env = dip.parse()
    env.save(file)
    c = DIPParse()
    c.argument_load(str(file))
    c.argument_print()
    assert c.execute() == "simulation.steps = 100\n"


def test_load_rejects_dipl_inputs(tmp_path):
    c = DIPParse()
    c.argument_load(str(tmp_path / "parameters.diph5"))
    with pytest.raises(RuntimeError, match="Conflicting DIP inputs"):
        c.argument_add("string", ["steps int = 100"])
    c = DIPParse()
    c.argument_add("string", ["steps int = 100"])
    with pytest.raises(RuntimeError, match="Conflicting DIP inputs"):
        c.argument_load(str(tmp_path / "parameters.diph5"))


def test_load_failure(tmp_path):
    c = DIPParse()
    c.argument_load(str(tmp_path / "missing.diph5"))
    with pytest.raises(RuntimeError):
        c.execute()


def test_save_failure(tmp_path):
    c = DIPParse()
    c.argument_add("string", ["steps int = 100"])
    c.argument_save(str(tmp_path / "missing" / "parameters.diph5"))
    with pytest.raises(RuntimeError):
        c.execute()


def test_generate_rejects_invalid_arguments(tmp_path):
    c = DIPParse()
    with pytest.raises(RuntimeError, match="Invalid export format"):
        c.argument_generate("toml", str(tmp_path / "parameters.toml"))
    with pytest.raises(RuntimeError, match="Invalid generate path"):
        c.argument_generate("json", "")

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

def test_add_project(tmp_path):

    (tmp_path / "parameters.dip").write_text("answer int = 42\n")
    (tmp_path / "DIPfile").write_text(
        "code[]\n"
        "  file = \"parameters.dip\"\n"
    )

    c = DIPParse()
    c.argument_add("project", [str(tmp_path / "DIPfile")])
    c.argument_request("answer")
    c.argument_value("integer")
    assert c.execute() == "42\n"

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
    
