# Command Line Interface for SciNumTools v3

`SNT` is the command-line interface for the `SciNumTools v3` scientific infrastructure platform.

It provides interface to:

PUQ — Physical Units & Quantities
DIP — Dimensional Input Parameters
MAT (planned) — Materials Properties

This executable is built together with the `SNT` library and provides the command-line interface implemented in the `src/snt/cli` and `include/snt/cli` directories.

## Example of use

``` bash
snt puq convert "12*statA" "A" -s ESU -S SI -Q "I"
# 4.00277e-9*A

snt dip parse \
    -f parameters.dip \
    -r "?family.father" \
    --print
# father = 184 cm
```

For more help and examples use the following:

``` bash
snt -h
snt puq -h
snt dip -h
```
