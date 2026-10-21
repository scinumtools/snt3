# DIPL source formatter

## Goal

Provide `snt fmt` as a predictable, source-preserving formatter for DIPL and
DIPfiles. The implementation lives in `snt-dip`, with a thin `snt-api` entry
point and CLI adapter. A later validator and linter can share source scanning
and diagnostics without making formatting depend on evaluation.

## Formatting contract

- DIPL indentation is always two spaces per level. It is not a configurable
  style setting; the first formatter pass preserves existing indentation
  rather than guessing the intended nesting of malformed source.
- By default, place one blank line between sibling nodes and keep each node's
  `?` and `!` annotations attached to it. Do not separate a parent from its
  first child.
- Normalize safe spaces around assignments and commas in inline arrays,
  including `!options`, `!labels`, and `!tags`. Preserve array order and the
  pairing of options and labels.
- Preserve comments, quoted strings, expressions, declaration order, and
  multiline scalar contents. Never evaluate or rewrite scientific values.
- Formatting must be idempotent. Unsupported or ambiguous constructs should
  remain unchanged rather than be guessed at.

## Configuration

The built-in DIPfile schema owns an optional `format` group. Its typed fields
and defaults are:

```dipl
format
  blank_lines_between_nodes = 1
  final_newline = true
```

These are formatter settings, not model parameters. An ordinary standalone
file uses defaults. A file under a project discovers the nearest DIPfile;
the DIPfile itself uses its own settings. Indentation remains fixed at two.
Future style options (for example array wrapping and line width) should be
added to this schema only when the formatter implements them.

## API and CLI

`snt::dip::tools::format(source, options)` returns text without touching the
filesystem. `snt::api` resolves project settings and supplies a file-level
entry point. `snt fmt <file>` prints formatted text; `--check` exits nonzero if
it differs; `--write` replaces the input. The CLI should accept several files
for check and write operations. No new third-party dependency is needed.

## Safety and verification

The formatter scans original lines, including blanks and comments. It does
not round-trip through `Environment`, which discards source layout. Tests
cover sibling spacing, annotations, comments, arrays, quoted delimiters,
multiline values, configuration, idempotence, and CLI exit codes. Follow-up
work may add a diff mode, configurable array wrapping, and a shared source
diagnostic layer for future validation and linting.
