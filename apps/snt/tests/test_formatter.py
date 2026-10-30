from conftest import checked


def test_formatter_preserves_values_and_bytes(cli, tmp_path):
    project = tmp_path / "project"
    project.mkdir()
    (project / "DIPfile").write_text("format\n  blank_lines_between_nodes = 0\n  final_newline = true\n")
    source = project / "parameters.dip"
    source.write_bytes(b"a int=1\nb int=2\n")
    before = checked(cli("dip", "parse", "--input", "file", source, "--print"))
    checked(cli("format", "--check", source), 1)
    checked(cli("format", "--write", source))
    assert source.read_bytes() == b"a int = 1\nb int = 2\n"
    assert checked(cli("dip", "parse", "--input", "file", source, "--print")) == before
    checked(cli("format", "--check", source))

    standalone = tmp_path / "standalone.dip"
    standalone.write_bytes(b"first int=1\nsecond int=2\n")
    written = tmp_path / "standalone-written.dip"
    written.write_bytes(standalone.read_bytes())
    checked(cli("format", "--write", written))
    assert written.read_bytes() == b"first int = 1\n\nsecond int = 2\n"
    assert cli("fmt", standalone).stdout == written.read_bytes()

    crlf = tmp_path / "crlf.dip"
    crlf.write_bytes(b"first int=1\r\nsecond int=2\r\n")
    crlf_written = tmp_path / "crlf-written.dip"
    crlf_written.write_bytes(crlf.read_bytes())
    checked(cli("format", "--write", crlf_written))
    assert crlf_written.read_bytes() == b"first int = 1\r\n\r\nsecond int = 2\r\n"
    result = cli("format", crlf)
    checked(result)
    assert result.stdout == crlf_written.read_bytes()
