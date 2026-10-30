import re
import shutil
import sys

from conftest import checked


def test_report_formats_and_pdf_adapter(cli, tmp_path):
    project = tmp_path / "project"
    project.mkdir()
    (project / "settings.dipl").write_text(
        '?descr "Reusable settings"\n?title "Schema & paper"\nspeed float = 2 m/s\n'
        '  ?descr "Flow & speed_#%"\n  !options [2, 3] m/s\n'
        '  !condition ({.} > 0 m/s)\n  ?rationale "Measured at startup"\n'
        '  ?recommended_range "2 to 3 m/s"\n  ?scientific_impact "Controls transport speed"\n'
    )
    (project / "parameters.dip").write_text(
        'physics : settings\ndouble_speed float = ({?physics.speed} * 2) m/s\n'
        'name str = "A&B_#%"\n  ?title "Value study"\n  ?doi "10.1/example_#"\n'
        'duration float = 12 h\nduration = 14 h\nmeasurements table = """\n'
        'temperature float K\ntime float s\n---\n295 0\n296 1\n"""\n'
    )
    (project / "DIPfile").write_text(
        'units[]\n  name = "custom_length"\n  unit = "2*m"\n'
        'schemas[]\n  name = "settings"\n  file = "settings.dipl"\n'
        'code[]\n  file = "parameters.dip"\n'
    )
    intro = project / "intro.tex"
    intro.write_text("This model uses \\textbf{measured} values.\n")
    manifest = project / "DIPfile"
    override = ("-i", "override_string", "physics.speed = 3 m/s")
    tex_file = tmp_path / "report.tex"
    checked(cli("report", "--project", manifest, *override, "--intro", intro,
                "--output", tex_file, "--title", "Demo & report", "--author", "Ada_Lovelace",
                "--date", "2026-09-28", "--report-version", "draft_1"))
    tex = tex_file.read_text()
    assert str(project / "parameters.dip") not in tex
    for fragment in (
        r"\begin{titlepage}", r"\tableofcontents", r"\usepackage{hyperref}",
        r"\section{Parameters}", r"Demo \& report", r"Ada\_Lovelace", "2026-09-28",
        r"draft\_1", "Introduction", "measured", "physics.speed", "3", "m/s",
        "Override at", "Modified at", "Modification", r"\section{Tables}",
        r"\sntnode{measurements}", "Column 1", "Reusable settings", r"Schema \& paper",
        r"Flow \& speed\_\#\%", "Value study", r"10.1/example\_\#", r"A\&B\_\#\%",
        "Sources", "Custom units", r"\sntnode{settings}", r"\sntnode{custom\_length}",
        "Parameter guide", r"\hyperlink{snt-parameter-", r"double\_speed",
        "Reads during evaluation", "?physics.speed", "Used by", "Supplied parameters",
        "Supplied by schema", "Applied overrides", "Allowed options", "Validation condition",
        "Author guidance", "Measured at startup", "Recommended range", "Controls transport speed",
        r"\sntnode{DIP0}", "2*m", "parameters.dip",
    ):
        assert fragment in tex, fragment
    match = re.search(r"Content hash & SHA-256: ([0-9a-f]+)\.\.\.", tex)
    assert match and len(match.group(1)) == 16
    assert "Overridden" not in tex and r"\section*{Hierarchy}" not in tex
    assert tex.find("name") < tex.find("physics.speed")

    for fmt in ("md", "rst", "html", "typ", "txt", "json"):
        output = tmp_path / f"report.{fmt}"
        checked(cli("report", "--project", manifest, *override, "--format", fmt,
                    "--output", output, "--date", "2026-09-28", "--report-version", "draft_1"))
        result = output.read_text()
        for fragment in ("physics.speed", "Override at", "Modified at", "Tables", "Column 1",
                         "Custom units", "Flow", "Parameter guide", "double", "Reads during evaluation",
                         "Supplied parameters", "Applied overrides", "Allowed options", "Validation condition",
                         "Measured at startup", "Recommended range", "Controls transport speed"):
            assert fragment in result, (fmt, fragment)
        if fmt == "json":
            assert len(re.search(r"SHA-256: ([0-9a-f]+)", result).group(1)) == 64
        if fmt == "html":
            assert re.search(r'href="#snt-parameter-[0-9]+"', result)
            assert re.search(r'id="snt-parameter-[0-9]+"', result)

    invalid = cli("report", "--project", manifest, "--intro", intro, "--format", "html",
                  "--output", tmp_path / "bad.html")
    assert invalid.returncode != 0 and b"--intro requires" in invalid.stderr

    snapshot = tmp_path / "snapshot.diph5"
    checked(cli("dip", "parse", "--project", manifest, *override, "--save", snapshot))
    loaded = tmp_path / "loaded.tex"
    checked(cli("report", "--load", snapshot, "--output", loaded))
    for fragment in ("physics.speed", "Override at", "Modified at", "Tables", "Column 1",
                     "Reusable settings", r"Schema \& paper", "Sources", "Custom units",
                     r"custom\_length", "Applied overrides", "Allowed options",
                     "Validation condition", "Recommended range"):
        assert fragment in loaded.read_text(), fragment

    inline = tmp_path / "inline.tex"
    checked(cli("report", "--input", "string", "count int = 7", "--output", inline))
    inline_text = inline.read_text()
    assert "count" in inline_text and re.search(r"Value.*7", inline_text)
    assert not re.search("Overridden|Override at", inline_text)

    if sys.platform != "win32":
        fake = tmp_path / "fake-pdflatex"
        captured = tmp_path / "compiled-source.tex"
        passes = tmp_path / "compiler-passes.txt"
        fake.write_text(
            '#!/bin/sh\nfor argument do\n  case "$argument" in\n'
            '    -output-directory=*) directory="${argument#-output-directory=}" ;;\n'
            '    *.tex) source="$argument" ;;\n  esac\ndone\n'
            f'cp "$source" "{captured}"\nprintf "pass\\n" >> "{passes}"\n'
            'printf "%%PDF-fake\\n" > "$directory/report.pdf"\n'
        )
        fake.chmod(0o700)
        checked(cli("report", "--project", manifest, *override, "--intro", intro,
                    "--format", "pdf", "--output", tmp_path / "fake.pdf", "--title", "Demo & report",
                    "--author", "Ada_Lovelace", "--date", "2026-09-28",
                    "--report-version", "draft_1", "--tex-compiler", fake))
        assert captured.read_text() == tex
        assert passes.read_text().splitlines() == ["pass", "pass"]

    missing = tmp_path / "missing.pdf"
    result = cli("report", "--load", snapshot, "--format", "pdf", "--output", missing,
                 "--tex-compiler", tmp_path / "no-such-compiler")
    assert result.returncode != 0 and b"unavailable" in result.stderr
    assert not missing.exists()
    pdflatex = shutil.which("pdflatex")
    if pdflatex:
        pdf = tmp_path / "report.pdf"
        checked(cli("report", "--project", manifest, "--format", "pdf", "--intro", intro,
                    "--output", pdf, "--tex-compiler", pdflatex))
        assert pdf.read_bytes()[:4] == b"%PDF"
