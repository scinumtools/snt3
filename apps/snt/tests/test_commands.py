import os
import re

from conftest import checked


def test_command_help_and_module_status(cli):
    def output(*args, status=0, contains=None):
        result = cli(*args)
        text = checked(result, status)
        combined = text + result.stderr.decode(errors="replace")
        if contains:
            assert contains in combined, (args, combined)
        return text

    for args, fragment in [
        (("--help",), "server"), (("--help",), "view"), (("--help",), "dmap"),
        (("dip", "--help"), "Commands:"), (("--help",), "Initialize a DIP project"),
        (("init", "--help"), "Command: init"), (("dip", "--help"), "Inspect one evaluated node as JSON"),
        (("dip", "--help"), "--relative-source-paths"), (("dip", "--help"), "Examples:"),
        (("dip", "--help"), "snt dip package --project DIPfile --output project.bundle.json"),
        (("dip", "parse", "--help"), "Command: parse"),
        (("dip", "compare", "--help"), "Compare two saved environments"),
        (("dip", "list", "--help"), "Select evaluated nodes by path query and tags"),
        (("dip", "preview", "--help"), "--override-file FILE"),
        (("hub", "--help"), "pinned code examples in a local SNT Hub workspace"),
        (("hub", "setup", "--help"), "override-file"),
    ]:
        output(*args, contains=fragment)

    for name, flag in [("report", "REPORT_ENABLED"), ("server", "SERVER_ENABLED"),
                       ("view", "VIEW_ENABLED"), ("dmap", "DMAP_ENABLED"),
                       ("mcp", "MCP_ENABLED")]:
        enabled = os.environ[flag] == "ON"
        symbol = r"\+" if enabled else "-"
        assert re.search(rf"{name}\s+{symbol}\s+", output("--help"))

    unknown = cli("unknown-command")
    assert unknown.returncode == 1 and b"Unknown module" in unknown.stderr
    for name, flag, title, disabled in [
        ("report", "REPORT_ENABLED", "Module: DIP Report", "ENABLE_SNT_REPORT=ON"),
        ("server", "SERVER_ENABLED", "Module: REST API server", "ENABLE_SNT_SERVER=ON"),
        ("dmap", "DMAP_ENABLED", "Module: PUQ dimension-map generator", "ENABLE_SNT_DMAP=ON"),
        ("view", "VIEW_ENABLED", "Module: Parameter viewer", "not included"),
    ]:
        result = cli(name, "--help")
        if os.environ[flag] == "ON":
            assert title in checked(result)
        else:
            assert result.returncode == 1 and disabled in result.stderr.decode()
    if os.environ["SERVER_ENABLED"] == "ON":
        for args, fragment in [(('server', '--port', '0'), 'Port must be'),
                               (('server', '--port'), 'Missing value'),
                               (('server', '--unknown'), 'Unknown option')]:
            result = cli(*args)
            assert result.returncode == 2 and fragment in (result.stdout + result.stderr).decode()
    if os.environ["VIEW_ENABLED"] == "ON":
        page = output("view")
        assert "Description:" in page and "Examples:" in page
    else:
        result = cli("view")
        assert result.returncode == 1 and b"not included" in (result.stdout + result.stderr)


def test_help_pages_have_consistent_sections(cli):
    pages = [
        ("--help",), ("puq",), ("puq", "--help"), ("dip",), ("dip", "--help"),
        *(('dip', sub, suffix) for sub in ('parse', 'compare', 'describe', 'schemas', 'sources',
                                          'override-contract', 'list', 'preview', 'package')
          for suffix in ('', '--help')),
        ("hub", "--help"),
        *(('hub', sub, suffix) for sub in ('fetch', 'setup') for suffix in ('', '--help')),
        *(("hub", sub, "--help") for sub in ('build', 'run')),
        ("init", "--help"),
        *((sub, suffix) for sub in ('setup', 'format', 'validate', 'lint', 'completion')
          for suffix in ('', '--help')),
    ]
    for module, flag in [("report", "REPORT_ENABLED"), ("server", "SERVER_ENABLED"),
                         ("dmap", "DMAP_ENABLED"), ("view", "VIEW_ENABLED"),
                         ("mcp", "MCP_ENABLED")]:
        if os.environ[flag] == "ON":
            pages.append((module, "--help"))
            if module in ("report", "view"):
                pages.append((module,))
    section_order = ["Usage", "Description", "Commands", "Modules", "Arguments", "Options", "Examples", "Notes"]
    for raw_args in pages:
        args = tuple(arg for arg in raw_args if arg)
        page = checked(cli(*args)).replace("\r\n", "\n")
        assert page.startswith("Scientific Numerical Tools (SNT)\n\n"), (args, page)
        assert page.count("Scientific Numerical Tools (SNT)") == 1, args
        headings = [line[:-1] for line in page.splitlines() if line.endswith(":") and line[:-1] in section_order]
        assert "Usage" in headings, (args, page)
        assert headings == sorted(headings, key=section_order.index), (args, headings)
