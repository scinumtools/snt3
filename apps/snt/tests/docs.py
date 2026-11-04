"""Check that the CLI exposes the existing DIPL sources as offline docs."""

import subprocess
import sys
import re
from pathlib import Path


executable = sys.argv[1]
root = Path(sys.argv[2])


def docs(*args):
    return subprocess.run(
        [executable, "dip", "docs", *args],
        capture_output=True, text=True, encoding="utf-8", check=False,
    )


chapters = docs("--list")
assert chapters.returncode == 0, chapters.stderr
assert re.search(r"^expressions +Expressions$", chapters.stdout, re.MULTILINE)
assert re.search(r"^grammar +Formal Grammar$", chapters.stdout, re.MULTILINE)
assert len({m.start() for line in chapters.stdout.splitlines()
            if (m := re.search(r"  \S", line))}) == 1

source = (root / "docs/dipl/syntax/expressions.md").read_text(encoding="utf-8")
chapter = docs("expressions")
assert chapter.returncode == 0, chapter.stderr
assert chapter.stdout == source

features = docs("expressions", "--list")
assert features.returncode == 0, features.stderr
assert re.search(r"^expressions#templates +Templates$", features.stdout, re.MULTILINE)
assert len({m.start() for line in features.stdout.splitlines()
            if (m := re.search(r"  \S", line))}) == 1

template = docs("expressions#templates")
assert template.returncode == 0, template.stderr
assert template.stdout.startswith("## Templates\n")
assert "Templates, or formatted" in template.stdout
assert template.stdout == docs("templates").stdout

grammar = docs("grammar")
assert grammar.returncode == 0, grammar.stderr
assert (root / "docs/dipl/grammar.ebnf").read_text(encoding="utf-8") in grammar.stdout

missing = docs("expressions#unknown-heading")
assert missing.returncode != 0
assert "Unknown heading" in missing.stderr
