from pathlib import Path
import json

import pytest

from scinumtools3.dip import DIP, Environment, ReportFormat


def test_generate_report_from_parsed_and_loaded_environment(tmp_path: Path):
    parser = DIP()
    parser.add_override_string("physics.speed = 3 m/s")
    parser.add_string('physics\n  speed float = 2 m/s\n    ?descr "Flow & speed"')
    env = parser.parse()

    intro = tmp_path / "intro.tex"
    intro.write_text(r"An \textbf{example} report.")
    tex = tmp_path / "report.tex"
    env.generate_report(ReportFormat.TEX, tex, input_label="Python example", intro_file=intro,
                      title="Study & report", author="Example_Team", date="2026-09-28",
                      version="v1.0")
    content = tex.read_text()
    assert "physics.speed" in content
    assert "Python example" in content
    assert "Flow \\& speed" in content
    assert "An \\textbf{example}" in content
    assert "Override at" in content
    assert "Study \\& report" in content
    assert "Example\\_Team" in content
    assert "\\tableofcontents" in content
    assert "\\section{Parameters}" in content
    assert "\\section{Hierarchy}" not in content

    formats = {
        ReportFormat.MARKDOWN: ("md", "physics.speed"),
        ReportFormat.RST: ("rst", "physics.speed"),
        ReportFormat.HTML: ("html", "Flow &amp; speed"),
        ReportFormat.TYPST: ("typ", "physics.speed"),
        ReportFormat.TEXT: ("txt", "physics.speed"),
        ReportFormat.JSON: ("json", '"schema":"briefpp/1"'),
    }
    for report_format, (extension, expected) in formats.items():
        file = tmp_path / f"report.{extension}"
        env.generate_report(report_format, file, title="Study & report", version="v1.0")
        result = file.read_text()
        assert expected in result
        assert "Flow & speed" in result or "Flow &amp; speed" in result or "Flow \\& speed" in result
        assert "Override at" in result
    json_report = json.loads((tmp_path / "report.json").read_text())
    assert json_report["metadata"]["title"] == "Study & report"
    with pytest.raises((ValueError, RuntimeError), match="LaTeX introduction"):
        env.generate_report(ReportFormat.HTML, tmp_path / "bad.html", intro_file=intro)

    snapshot = tmp_path / "run.diph5"
    env.save(snapshot)
    loaded = Environment()
    loaded.load(snapshot)
    loaded_tex = tmp_path / "loaded.tex"
    loaded.generate_report(ReportFormat.TEX, loaded_tex)
    assert "DIPH5 snapshot" in loaded_tex.read_text()
    assert "Override at" in loaded_tex.read_text()

    with pytest.raises(RuntimeError, match="unavailable"):
        loaded.generate_report(ReportFormat.PDF, tmp_path / "missing.pdf",
                             tex_compiler=str(tmp_path / "no-compiler"))
