"""Check that the prebuilt PDF matches this source distribution's versions."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from versions import resolve

HERE = Path(__file__).resolve().parent
PDF = HERE / "dipl-cheatsheet.pdf"
MANIFEST = HERE / "dipl-cheatsheet.manifest.json"


def check() -> None:
    if not PDF.is_file() or not MANIFEST.is_file():
        raise RuntimeError("The prebuilt DIPL cheat sheet and its manifest are required")
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    versions = resolve(require_commit=False)
    if manifest["versions"] != versions:
        raise RuntimeError("The prebuilt DIPL cheat sheet has stale language or SNT3 versions")
    if manifest["pdf_sha256"] != hashlib.sha256(PDF.read_bytes()).hexdigest():
        raise RuntimeError("The prebuilt DIPL cheat sheet PDF differs from its manifest")


if __name__ == "__main__":
    check()
    print("Prebuilt DIPL cheat sheet versions and checksum are current")
