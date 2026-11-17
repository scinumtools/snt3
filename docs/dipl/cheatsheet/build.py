"""Build and validate the canonical DIPL/PUEL quick-reference PDF."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

from verify import verify
from versions import ROOT, metadata_yaml, resolve

HERE = Path(__file__).resolve().parent
OUTPUT = HERE / "dipl-cheatsheet.pdf"
INPUTS = [HERE / name for name in ("cheatsheet.md", "template.tex", "metadata.yaml", "build.py", "verify.py", "versions.py")]
INPUTS += [ROOT / name for name in (
    "docs/dipl/specification.md", "docs/puel/specification.md",
    "docs/diph5/specification.md", "src/snt/dip/hdf5/io.cpp", "settings.env",
)]


def _run(args: list[str], **kwargs) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, cwd=ROOT, check=True, text=True, **kwargs)


def _page_count(pdf: Path) -> int:
    expression = f"({pdf.as_posix()}) (r) file runpdfbegin pdfpagecount == quit"
    result = _run(["gs", "-q", "-dNOSAFER", "-dNODISPLAY", "-c", expression], capture_output=True)
    return int(result.stdout.strip())


def _pdf_text(pdf: Path) -> str:
    return _run(["gs", "-q", "-sDEVICE=txtwrite", "-o", "-", str(pdf)], capture_output=True).stdout


def _check_a4(pdf: Path, pages: int) -> None:
    for page in range(1, pages + 1):
        expression = f"({pdf.as_posix()}) (r) file runpdfbegin {page} pdfgetpage /MediaBox get == quit"
        box = _run(["gs", "-q", "-dNOSAFER", "-dNODISPLAY", "-c", expression], capture_output=True).stdout
        numbers = [float(value) for value in re.findall(r"[0-9]+(?:\.[0-9]+)?", box)]
        if len(numbers) != 4 or abs(numbers[2] - numbers[0] - 595.276) > 0.2 \
                or abs(numbers[3] - numbers[1] - 841.89) > 0.2:
            raise RuntimeError(f"Cheat-sheet page {page} is not A4 portrait: {box.strip()}")


def build(output: Path = OUTPUT) -> Path:
    versions = resolve()
    verified = verify()
    build_dir = ROOT / "build/dipl-cheatsheet"
    build_dir.mkdir(parents=True, exist_ok=True)
    generated_metadata = build_dir / "versions.yaml"
    generated_metadata.write_text(metadata_yaml(versions), encoding="utf-8")
    fingerprint = hashlib.sha256()
    for path in INPUTS:
        fingerprint.update(path.read_bytes())
    fingerprint.update(generated_metadata.read_bytes())
    signature = fingerprint.hexdigest()
    stamp = build_dir / "dipl-cheatsheet.sha256"
    if (output.is_file() and output.with_suffix(".manifest.json").is_file()
            and stamp.is_file() and stamp.read_text().strip() == signature):
        print(f"Cheat sheet is current: {output} ({verified} verified examples)")
        return output
    temporary = build_dir / "dipl-cheatsheet.pdf"
    environment = os.environ.copy()
    environment["TEXMFVAR"] = str(build_dir / "texmf-var")
    environment["TEXMFCONFIG"] = str(build_dir / "texmf-config")
    environment["XDG_CACHE_HOME"] = str(build_dir / "cache")
    environment.setdefault("SOURCE_DATE_EPOCH", _run(["git", "show", "-s", "--format=%ct", "HEAD"],
                                                       capture_output=True).stdout.strip())
    _run([
        "pandoc", "--from=markdown+fenced_code_attributes", "--pdf-engine=lualatex",
        "--template", str(HERE / "template.tex"), "--listings",
        "--metadata-file", str(HERE / "metadata.yaml"),
        "--metadata-file", str(generated_metadata),
        "--output", str(temporary), str(HERE / "cheatsheet.md"),
    ], env=environment)
    pages = _page_count(temporary)
    if not 1 <= pages <= 2:
        raise RuntimeError(f"The cheat sheet has {pages} pages; expected at most two")
    _check_a4(temporary, pages)
    extracted = _pdf_text(temporary)
    for name in ("dipl", "puel", "diph5", "snt3"):
        if versions[name] not in extracted:
            raise RuntimeError(f"The PDF is missing {name.upper()} version {versions[name]}")
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary.replace(output)
    manifest = {
        "versions": {name: versions[name] for name in ("dipl", "puel", "diph5", "snt3")},
        "git_commit": versions["commit"],
        "pdf_sha256": hashlib.sha256(output.read_bytes()).hexdigest(),
    }
    output.with_suffix(".manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    stamp.write_text(signature + "\n", encoding="utf-8")
    print(f"Built {output} ({pages} A4 pages; {verified} verified examples)")
    return output


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    args = parser.parse_args()
    try:
        build(args.output)
    except (RuntimeError, subprocess.CalledProcessError, FileNotFoundError, ValueError) as error:
        print(f"Cheat-sheet build failed: {error}", file=sys.stderr)
        raise SystemExit(1)
