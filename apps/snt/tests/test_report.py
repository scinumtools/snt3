import re
import shutil
import sys
from html import unescape
from pathlib import Path
from urllib.parse import unquote, urlsplit

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


def test_report_docs_preserves_readmes_and_excludes_overrides(cli, tmp_path):
    root = tmp_path / "settings"
    root.mkdir()
    (root / "README.md").write_text(
        "# Example settings\n\nSee [runtime](runtime/README.md) and [build](build/README.md).\n"
    )
    build = root / "build"
    build.mkdir()
    (build / "README.md").write_text("# Build\n\nCompiled constants.\n")
    (build / "DIPfile").write_text('code[]\n  file = "limits.dip"\n')
    (build / "limits.dip").write_text('limit int = 7\n')
    project = root / "runtime"
    project.mkdir()
    (project / "README.md").write_text("# Runtime\n\nEdit values in `overrides.dip`.\n")
    (project / "schemas").mkdir()
    (project / "schemas" / "README.md").write_text("# Schemas\n\nTyped defaults.\n")
    (project / "schemas" / "controls.dip").write_text(
        '?descr "Control settings"\nlevel int = 2\n  ?descr "Display level"\n'
        '  !options [1, 2, 3]\n  !condition ({.} >= 1)\n  !setup\nsize int = 5\n'
    )
    (project / "values.dip").write_text('?descr "Runtime control values"\ncontrols : controls_schema\n  level = 3\n')
    (project / "switches.dip").write_text(
        '?descr "Operational switches"\n'
        'first bool = true\n  ?descr "Override to enable the first mode"\n'
        '  ?category "Operations"\n  ?visibility "basic"\n'
        'second bool = false\nthird bool = true\n'
        'data_path str = "./"\n  ?descr "Set a local path in an override source"\n'
    )
    (project / "many.dip").write_text(
        '\n'.join(f'option_{index} int = {index}' +
                  ('\n  ?visibility "basic"' if index == 7 else '') for index in range(9)) + '\n'
    )
    (project / "overrides.dip").write_text('this invalid local override must be ignored\n')
    (project / "DIPfile").write_text(
        'schemas[]\n  name = "controls_schema"\n  file = "schemas/controls.dip"\n'
        'code[]\n  file = "values.dip"\n'
        'code[]\n  file = "switches.dip"\n'
        'code[]\n  file = "many.dip"\n'
        'overrides[]\n  file = "overrides.dip"\n'
    )
    output = tmp_path / "docs"
    checked(cli("report", "docs", "--root", root, "--output-dir", output,
                "--format", "md"))
    assert "[runtime](runtime/index.md)" in (output / "index.md").read_text()
    assert "[build](build/index.md)" in (output / "index.md").read_text()
    assert "Compiled constants." in (output / "build" / "index.md").read_text()
    assert "limit" in (output / "build" / "limits.dip.md").read_text()
    assert "Typed defaults." in (output / "runtime" / "schemas" / "index.md").read_text()
    schema = (output / "runtime" / "schemas" / "controls.dip.md").read_text()
    assert "**Default:** `2`" in schema
    values = (output / "runtime" / "values.dip.md").read_text()
    assert "controls.level" in values and "**Value:** `3`" in values
    assert "controls.size" in values and "**Value:** `5`" in values
    assert "invalid local override" not in values
    assert "controls.level = 3" in values
    assert "Runtime control values" in values
    assert "controls.level" in (output / "search.md").read_text()
    assert "Guided setup" in (output / "index.md").read_text()
    assert "Examples to explore" not in (output / "index.md").read_text()
    switches = (output / "runtime" / "switches.dip.md").read_text()
    assert "All 4 evaluated settings are listed below" in switches
    assert all(f"`{name}` =" in switches for name in ("first", "second", "third", "data_path"))
    assert "data_path = \"./\"" not in switches
    assert "**Override guidance:** Assign a value suitable for your environment" in switches
    assert "Override syntax using the current value" in switches
    many = (output / "runtime" / "many.dip.md").read_text()
    assert "1 of 9 evaluated settings are shown below" in many
    assert "- `option_7` = `7`" in many
    assert "[View all settings](#evaluated-settings)" in many
    search = (output / "search.md").read_text()
    assert "Category: Operations" in search and "Visibility: basic" in search

    if shutil.which("pandoc") and shutil.which("xelatex"):
        cheatsheet = Path(__file__).resolve().parents[3] / "docs/dipl/cheatsheet/dipl-cheatsheet.pdf"
        checked(cli("report", "docs", "--root", root, "--output-dir", output,
                    "--format", "all", "--cheatsheet-pdf", cheatsheet))
        assert (output / "index.html").is_file()
        pages = {page.resolve(): page.read_text() for page in output.rglob("*.html")}
        anchors = {page: set(re.findall(r'id="([^"]+)"', content))
                   for page, content in pages.items()}
        for page, content in pages.items():
            for raw in re.findall(r'href="([^"]+)"', content):
                link = urlsplit(unescape(raw))
                if link.scheme or link.netloc:
                    continue
                target = (page.parent / unquote(link.path)).resolve() if link.path else page
                assert target.exists(), (page, raw)
                if link.fragment and target in anchors:
                    assert unquote(link.fragment) in anchors[target], (page, raw)
        assert (output / "settings-reference.pdf").read_bytes().startswith(b"%PDF")
        book = (output / "settings-reference.md").read_text()
        assert "# Build" in book and "# Runtime" in book
        assert "Settings overview" in book and "controls.level" in book
        assert "## Evaluated settings" not in book
        ids = set(re.findall(r"\{#([^\s}]+)", book))
        refs = set(re.findall(r"\]\(#([^)]*)\)", book))
        assert refs <= ids
        assert "\\includepdf" in book
        assert (output / "settings-reference-cheatsheet.pdf").read_bytes() == cheatsheet.read_bytes()
        checked(cli("report", "docs", "--root", root, "--output-dir", output,
                    "--format", "pdf", "--pdf-detail", "full"))
        full_book = (output / "settings-reference.md").read_text()
        assert "controls.size" in full_book and "[]{#setting-" in full_book
        full_ids = set(re.findall(r"\{#([^\s}]+)", full_book))
        full_refs = set(re.findall(r"\]\(#([^)]*)\)", full_book))
        assert full_refs <= full_ids


def test_report_docs_fallback_navigation_uses_described_guided_fields(cli, tmp_path):
    root = tmp_path / "configuration"
    root.mkdir()
    (root / "README.md").write_text("# Example configuration\n\nA standalone project.\n")
    (root / "schema.dip").write_text(
        'main int = 1\n  ?descr "Primary control"\n  !setup\n'
        'private int = 2\n  !setup\n'
        'expert int = 3\n  ?descr "Expert control"\n  ?visibility "advanced"\n  !setup\n'
    )
    (root / "values.dip").write_text('controls : control_schema\n')
    (root / "DIPfile").write_text(
        'schemas[]\n  name = "control_schema"\n  file = "schema.dip"\n'
        'code[]\n  file = "values.dip"\n'
    )
    output = tmp_path / "docs"
    checked(cli("report", "docs", "--root", root, "--output-dir", output, "--format", "md"))
    index = (output / "index.md").read_text()
    examples = index.split("## Examples to explore", 1)[1].split("## Contents", 1)[0]
    assert "All 1 guided settings" in examples
    assert "controls.main" in examples and "Primary control" in examples
    assert "controls.private" not in examples and "controls.expert" not in examples
    assert "Browse every guided setting" in examples
    search = (output / "search.md").read_text()
    assert "controls.private" in search and "controls.expert" in search
    if shutil.which("pandoc") and shutil.which("xelatex"):
        checked(cli("report", "docs", "--root", root, "--output-dir", output, "--format", "all"))
        assert (output / "index.html").is_file()
        assert (output / "settings-reference.pdf").read_bytes().startswith(b"%PDF")
        book = (output / "settings-reference.md").read_text()
        ids = set(re.findall(r"\{#([^\s}]+)", book))
        refs = set(re.findall(r"\]\(#([^)]*)\)", book))
        assert refs <= ids


def test_report_docs_places_schema_defaults_with_initial_source(cli, tmp_path):
    root = tmp_path / "model"
    root.mkdir()
    (root / "schema.dipl").write_text('first int = 1\nsecond int = 2\n')
    (root / "base.dip").write_text('group : shape\n  first = 3\n')
    (root / "later.dip").write_text('group.first = 4\n')
    (root / "DIPfile").write_text(
        'schemas[]\n  name = "shape"\n  file = "schema.dipl"\n'
        'code[]\n  file = "base.dip"\n'
        'code[]\n  file = "later.dip"\n'
    )
    output = tmp_path / "docs"
    checked(cli("report", "docs", "--root", root, "--output-dir", output, "--format", "md"))
    base = (output / "base.dip.md").read_text()
    later = (output / "later.dip.md").read_text()
    assert "### `group.second`" in base
    assert "### `group.first`" not in base
    assert "### `group.first`" in later
    assert "**Value assigned in:** `later.dip:1`" in later


def test_report_docs_handles_link_values_with_registered_overrides(cli, tmp_path):
    root = tmp_path / "configuration"
    root.mkdir()
    (root / "values.dip").write_text(
        'value int = 1\nalias link = {?value}\n  ?descr "Override the target value, not this link"\n'
    )
    (root / "overrides.dip").write_text("")
    (root / "DIPfile").write_text(
        'code[]\n  file = "values.dip"\n'
        'overrides[]\n  file = "overrides.dip"\n'
    )
    output = tmp_path / "docs"
    checked(cli("report", "docs", "--root", root, "--output-dir", output, "--format", "md"))
    page = (output / "values.dip.md").read_text()
    assert "### `alias`" in page and "**Links to:**" in page
    assert "Override syntax using the current value" not in page
