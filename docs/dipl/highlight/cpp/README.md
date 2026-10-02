# DIPL C++ highlighter

`dipl_highlighter.h` exposes a C++17 tokenizer and RGB palette without ImGui,
file I/O, or other runtime dependencies. It accepts source text and returns
lines of DIPL token spans. Callers choose how to draw or export them.

```cpp
#include <dipl_highlighter.h>

auto lines = snt::dipl::highlight::tokenize("length float = 2.5 m\n");
for (const auto& line : lines)
    for (const auto& span : line.spans)
        draw(span.text, snt::dipl::highlight::color(span.kind));
```

With CMake, add this directory as a subdirectory and link
`snt::dipl_highlighter`, or configure this directory as a standalone project.
The SNT3 viewer uses this target. The lexer preserves each displayed line's
text and recognizes common DIPL names, types, values, units,
references, directives, comments, and multiline strings. Its palette matches
the DIPL Pygments style in `../pygments/style_lexer.py`.

This is a lightweight native lexer. The Pygments lexer remains a separate
implementation, so edge-case tokenization can differ. Use the shared
`../highlighting-test.dipl` fixture when changing either lexer.
