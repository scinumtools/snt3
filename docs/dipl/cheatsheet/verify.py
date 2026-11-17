"""Check marked cheat-sheet examples with the installed SNT3 parsers."""

from __future__ import annotations

import re
import sys
from pathlib import Path

SOURCE = Path(__file__).with_name("cheatsheet.md")
START = re.compile(r"^```(?P<language>dipl|puel|bash)\s+\{\.(?P<kind>verify|fragment|illustrative|invalid)(?:\s+[^}]*)?\}\s*$")


def examples(source: str):
    lines = source.splitlines()
    index = 0
    while index < len(lines):
        match = START.fullmatch(lines[index])
        if not match:
            if lines[index].startswith("```"):
                raise RuntimeError(f"Unclassified code fence at line {index + 1}")
            index += 1
            continue
        first = index + 1
        index += 1
        body = []
        while index < len(lines) and lines[index] != "```":
            body.append(lines[index])
            index += 1
        if index == len(lines):
            raise RuntimeError(f"Unclosed code fence at line {first}")
        yield first, match["language"], match["kind"], "\n".join(body)
        index += 1


def verify() -> int:
    try:
        from scinumtools3.dip import DIP
        from scinumtools3.puq import Quantity
    except ImportError as error:
        raise RuntimeError("Example verification requires the compiled scinumtools3 Python package") from error

    verified = 0
    for line, language, kind, body in examples(SOURCE.read_text(encoding="utf-8")):
        if kind == "illustrative":
            continue
        if kind == "fragment":
            # A partial declaration can be checked as a child of a temporary group.
            if language != "dipl":
                raise RuntimeError(f"Unsupported {language} fragment at line {line}")
            body = "fragment\n" + "\n".join("  " + row for row in body.splitlines())
        try:
            if language == "dipl":
                parser = DIP()
                parser.add_string(body)
                parser.parse()
            elif language == "puel":
                for expression in body.splitlines():
                    if expression.strip() and not expression.lstrip().startswith("#"):
                        Quantity(expression.strip()).to_string()
            else:
                raise RuntimeError(f"Unsupported verified language {language} at line {line}")
        except Exception as error:
            if kind == "invalid":
                verified += 1
                continue
            raise RuntimeError(f"{language} example at line {line} failed: {error}") from error
        if kind == "invalid":
            raise RuntimeError(f"Invalid example at line {line} unexpectedly passed")
        verified += 1
    if verified < 5:
        raise RuntimeError(f"Only {verified} examples are verified; at least five are required")
    return verified


if __name__ == "__main__":
    try:
        print(f"Verified {verify()} DIPL/PUEL examples")
    except RuntimeError as error:
        print(error, file=sys.stderr)
        raise SystemExit(1)
