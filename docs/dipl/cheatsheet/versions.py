"""Resolve the versions printed on the DIPL/PUEL quick reference."""

from __future__ import annotations

import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SEMVER = r"[0-9]+\.[0-9]+\.[0-9]+"


def _match(path: Path, pattern: str, label: str) -> str:
    matches = re.findall(pattern, path.read_text(encoding="utf-8"), re.MULTILINE)
    if len(matches) != 1:
        raise RuntimeError(f"Cannot resolve one authoritative {label} version from {path}")
    return matches[0] if isinstance(matches[0], str) else ".".join(matches[0])


def resolve(*, require_commit: bool = True) -> dict[str, str]:
    versions = {
        "dipl": _match(ROOT / "docs/dipl/specification.md", rf'^version:\s*["\']?({SEMVER})', "DIPL"),
        "puel": _match(ROOT / "docs/puel/specification.md", rf'^version:\s*["\']?({SEMVER})', "PUEL"),
        "diph5": _match(ROOT / "docs/diph5/specification.md", rf'^version:\s*["\']?({SEMVER})', "DIPH5"),
        "snt3": _match(ROOT / "settings.env", rf'^CODE_VERSION=({SEMVER})$', "SNT3"),
    }
    schema_parts = []
    for part in ("Major", "Minor", "Patch"):
        schema_parts.append(_match(ROOT / "src/snt/dip/hdf5/io.cpp",
                                   rf'write_scalar<uint32_t>\(output, "_DIPH5_Schema_{part}", H5T_NATIVE_UINT32, ([0-9]+)\)',
                                   f"DIPH5 {part.lower()}"))
    if versions["diph5"] != ".".join(schema_parts):
        raise RuntimeError("DIPH5 specification version differs from the runtime writer")
    if not require_commit:
        return versions
    try:
        versions["commit"] = subprocess.check_output(
            ["git", "rev-parse", "--short=12", "HEAD"], cwd=ROOT, text=True
        ).strip()
    except (subprocess.CalledProcessError, FileNotFoundError) as error:
        raise RuntimeError("A Git commit hash is required for PDF metadata") from error
    if not re.fullmatch(r"[0-9a-f]{12}", versions["commit"]):
        raise RuntimeError("Cannot resolve a 12-character Git commit hash")
    return versions


def metadata_yaml(versions: dict[str, str]) -> str:
    version_line = " · ".join(f"{name.upper()} {versions[name]}" for name in ("dipl", "puel", "diph5", "snt3"))
    return (
        f'versions: "{version_line}"\n'
        f'git_commit: "{versions["commit"]}"\n'
        f'pdf_subject: "DIPL and PUEL quick reference; {version_line}; git {versions["commit"]}"\n'
    )
