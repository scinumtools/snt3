#!/usr/bin/env python3
"""Embed the existing DIPL specification chapters in the snt executable."""

import argparse
import hashlib
import json
import re
from pathlib import Path


HEADING = re.compile(r"^ {0,3}(#{1,6})[ \t]+(.+?)[ \t]*#*[ \t]*$")
FENCE = re.compile(r"^ {0,3}(`{3,}|~{3,})")


def quoted(value):
    return json.dumps(value, ensure_ascii=False)


def raw(value):
    marker = "D" + hashlib.sha256(value.encode("utf-8")).hexdigest()[:12]
    while ")" + marker + '"' in value:
        marker = "D" + hashlib.sha256(marker.encode()).hexdigest()[:12]
    return f'R"{marker}({value}){marker}"'


def chapters(manifest, root):
    in_files = False
    paths = []
    for line in manifest.read_text(encoding="utf-8").splitlines():
        if line == "input-files:":
            in_files = True
            continue
        if in_files and line and not line.startswith(" "):
            break
        if in_files:
            match = re.fullmatch(r"  - (docs/dipl/(?:specification|syntax/[^/]+)\.md)", line)
            if match:
                paths.append(root / match.group(1))
    if not paths or not paths[0].name == "specification.md":
        raise ValueError("DIPL chapter list missing from pandoc.yaml")
    for path in paths:
        if not path.is_file():
            raise FileNotFoundError(path)
        yield path.stem, path.relative_to(root).as_posix(), path.read_text(encoding="utf-8")
    grammar = root / "docs/dipl/grammar.ebnf"
    yield "grammar", grammar.relative_to(root).as_posix(), "# Formal Grammar\n\n```ebnf\n" + grammar.read_text(encoding="utf-8") + "```\n"


def slug(title):
    title = re.sub(r"[`*_]", "", title).lower()
    title = re.sub(r"[^\w\s-]", "", title, flags=re.UNICODE)
    return re.sub(r"\s+", "-", title.strip())


def headings(content):
    result = []
    seen = {}
    fence_char = ""
    fence_size = 0
    offset = 0
    for line in content.splitlines(keepends=True):
        marker = FENCE.match(line)
        if marker:
            found = marker.group(1)
            if not fence_char:
                fence_char, fence_size = found[0], len(found)
            elif found[0] == fence_char and len(found) >= fence_size:
                fence_char, fence_size = "", 0
        elif not fence_char:
            match = HEADING.match(line.rstrip("\r\n"))
            if match:
                title = match.group(2).strip()
                base = slug(title)
                if base:
                    count = seen.get(base, 0)
                    seen[base] = count + 1
                    result.append([base if count == 0 else f"{base}-{count}", title,
                                   len(match.group(1)), offset, len(content.encode("utf-8"))])
        offset += len(line.encode("utf-8"))
    for index, heading in enumerate(result):
        for next_heading in result[index + 1:]:
            if next_heading[2] <= heading[2]:
                heading[4] = next_heading[3]
                break
    return result


def generate(manifest, root):
    docs = list(chapters(manifest, root))
    names = [key for key, _, _ in docs]
    if len(names) != len(set(names)):
        raise ValueError("DIPL chapter keys must be unique")
    lines = ["// Generated from docs/dipl by generate_dip_docs.py. Do not edit.\n",
             "static const Doc kDocs[] = {\n"]
    all_headings = []
    for key, source, content in docs:
        found = headings(content)
        if not found:
            raise ValueError(f"No Markdown heading in {source}")
        lines.append(f"    {{{quoted(key)}, {quoted(found[0][1])}, {quoted(source)}, {raw(content)}}},\n")
        all_headings.extend((key, *heading) for heading in found)
    lines.append("};\n\nstatic const Heading kHeadings[] = {\n")
    for key, name, title, _, start, end in all_headings:
        lines.append(f"    {{{quoted(key)}, {quoted(name)}, {quoted(title)}, {start}, {end}}},\n")
    lines.append("};\n")
    return "".join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    content = generate(args.root / "docs/dipl/pandoc.yaml", args.root)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text(encoding="utf-8") != content:
        args.output.write_text(content, encoding="utf-8")


if __name__ == "__main__":
    main()
