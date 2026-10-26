# Guided DIPL setup

Try the interactive setup command from the repository root:

```sh
build/bin/snt setup examples/dip/GuidedSetup/DIPfile --output /tmp/snt-guided-setup.dip
```

The example marks user inputs with `!setup`. It demonstrates a boolean prompt,
human-readable `!options`/`!labels`, constrained integer and unit-bearing
numeric inputs, a formatted text input, a typed array expression, and a
required value without a default. The derived value is omitted from the
Each answer is checked immediately by the normal DIP parser. Invalid values
show the parser error and return to that prompt. The completed answers are
validated together again before writing a DIPL override file; the example
project is left unchanged.

Apply the generated override when evaluating the project:

```sh
build/bin/snt dip parse --project examples/dip/GuidedSetup/DIPfile \
  --input override_file /tmp/snt-guided-setup.dip --print
```
