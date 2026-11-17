"""Check the specification appendix and published Sphinx download."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path

from build import _page_count
from versions import ROOT, resolve


def check(html_dir: Path) -> None:
    flyer = ROOT / "docs/dipl/cheatsheet/dipl-cheatsheet.pdf"
    spec = ROOT / "build/dipl-specification.pdf"
    published = html_dir / "dipl-cheatsheet.pdf"
    index = html_dir / "dipl/index.html"
    if not published.is_file() or published.read_bytes() != flyer.read_bytes():
        raise RuntimeError("Sphinx did not publish the prebuilt cheat-sheet PDF")
    for language in ("dipl", "puel", "diph5"):
        filename = f"{language}-specification.pdf"
        built = (ROOT / "build" / filename).read_bytes()
        for target in (
            ROOT / "docs" / language / filename,
            ROOT / "docs/source/_static" / filename,
            html_dir / "_static" / filename,
        ):
            if not target.is_file() or target.read_bytes() != built:
                raise RuntimeError(f"The {language.upper()} specification copy is missing or stale: {target}")
    html = index.read_text(encoding="utf-8")
    if "../dipl-cheatsheet.pdf" not in html or "../_static/dipl-specification.pdf" not in html:
        raise RuntimeError("The DIPL page is missing a reference download link")
    last = _page_count(spec)
    text = subprocess.run([
        "gs", "-q", "-dFirstPage=" + str(last), "-dLastPage=" + str(last),
        "-sDEVICE=txtwrite", "-o", "-", str(spec),
    ], cwd=ROOT, check=True, capture_output=True, text=True).stdout
    if "DIPL" not in text or "PUEL" not in text or "quickreference" not in "".join(text.lower().split()):
        raise RuntimeError("The specification does not end with the cheat-sheet appendix")
    for name, version in resolve(require_commit=False).items():
        if version not in text:
            raise RuntimeError(f"The specification appendix is missing {name.upper()} {version}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("html_dir", type=Path)
    check(parser.parse_args().html_dir)
    print("Specification appendix and Sphinx download are current")
