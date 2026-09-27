from pathlib import Path

import pytest

from scinumtools3.dip import DIP, Environment, ReportFormat


def test_generate_docs_from_parsed_and_loaded_environment(tmp_path: Path):
    parser = DIP()
    parser.add_override_string("physics.speed = 3 m/s")
    parser.add_string('physics\n  speed float = 2 m/s\n    ?descr "Flow & speed"')
    env = parser.parse()

    intro = tmp_path / "intro.tex"
    intro.write_text(r"An \textbf{example} report.")
    tex = tmp_path / "report.tex"
    env.generate_docs(ReportFormat.TEX, tex, input_label="Python example", intro_file=intro)
    content = tex.read_text()
    assert "physics.speed" in content
    assert "Python example" in content
    assert "Flow \\& speed" in content
    assert "An \\textbf{example}" in content
    assert "Override at" in content

    snapshot = tmp_path / "run.diph5"
    env.save(snapshot)
    loaded = Environment()
    loaded.load(snapshot)
    loaded_tex = tmp_path / "loaded.tex"
    loaded.generate_docs(ReportFormat.TEX, loaded_tex)
    assert "DIPH5 snapshot" in loaded_tex.read_text()
    assert "Override at" in loaded_tex.read_text()

    with pytest.raises(RuntimeError, match="unavailable"):
        loaded.generate_docs(ReportFormat.PDF, tmp_path / "missing.pdf",
                             tex_compiler=str(tmp_path / "no-compiler"))
