DIPL describes typed scientific parameters; PUEL supplies physical quantities
and units. Indent by two spaces per child level. A `#` starts a comment.
Values are evaluated and validated before an application reads them.

## Declarations and types

`name type = value [unit]` defines a value. A declaration without `=` may be
completed later. A later assignment to the same path modifies it compatibly.

```dipl {.verify}
simulation
  title str = "Heat-flow run"
  enabled bool = true
  steps uint = 500
  timestep float = 0.5 ms
```

**Scalar types:** `bool`, `int`, `uint`, `float`, `str`; fixed-width forms include
`int16`, `uint64`, `float32`, and `float64`. `none` is an explicit empty value.
Arrays put a shape after the type: `[3]`, `[2,2]`, `[3:]` (minimum three), or
`[:4]` (maximum four).

```dipl {.verify}
mass float kg
mass = 750 g
velocity float[3] = [1.0, 0.0, 0.0] m/s
labels str[2] = ["inlet", "outlet"]
```

Groups use indentation. A named item is `group[key]`; `group[]` appends a list
item. Dotted names denote the same hierarchical path.

```dipl {.verify}
materials[water]
  density float = 998.2 kg/m3
materials[air]
  density float = 1.225 kg/m3
sensors[]
  name str = "inlet"
sensors[]
  name str = "outlet"
```

## References and expressions

`{?path}` reads a local value; `{source?path}` reads a registered source.
`{.}` refers to the current value inside `!condition`. Parentheses form
expressions; units stay attached during arithmetic.

```dipl {.verify}
geometry
  length float = 2 m
  width float = 40 cm
area float = ({?geometry.length} * {?geometry.width}) m2
```

Use `{?path.}` to select a subtree and `{source}` for raw source text. A leading
dot in a reference is relative to the current hierarchy; two dots reach the
parent. An unresolved reference or incompatible dimension is an error.

## Validation and guidance

`!options` restricts discrete values; `!condition` checks a logical expression
after evaluation. `!format` applies a regular expression to a string.
`!setup` marks a value for the guided setup command. Metadata beginning with
`?`, such as `?descr`, documents a node without changing its value.

```dipl {.verify}
method str = "rk4"
  !options ["euler", "rk4", "adaptive"]
  !setup
steps int = 500
  !condition ({.} > 0)
  ?descr "Positive number of integration steps"
```

Cross-field validation can refer to another value. Use compatible units in
comparisons.

```dipl {.verify}
limits
  maximum float = 10 m/s
speed float = 3 m/s
  !condition ({.} <= {?limits.maximum})
```

\columnbreak

## Conditional content

`@if`, `@elif`, `@else`, and optional `@end` select declarations based on
evaluated expressions. Clause indentation does not add to the final node path.

```dipl {.verify}
mode str = "fast"
@if ({?mode} == "fast")
  iterations int = 100
@else
  iterations int = 1000
@end
```

## PUEL notation

Standalone PUEL expressions are compact and whitespace-free. `*` joins a
number and unit; `/` divides unit factors; a trailing integer is an exponent
(`m2`, `s-2`). Prefixes such as `k` and `m` scale a unit. Addition belongs to
the PUQ calculator, not to a standalone PUEL literal.

```puel {.verify}
2.5*m
23*kg*m2/s2
[2,3,4]*cm
```

An uncertainty can be written in parentheses after the value. Conversion
requires compatible dimensions.

```puel {.verify}
23.45(12)*km
```

**Typical units:** length `m`, time `s`, mass `kg`, speed `m/s`, force `N`,
energy `J`. DIPL permits a space before the unit in a declaration; the
standalone PUEL form uses `*`.

## Sources, schemas and overrides

`$source` registers another DIPL file or raw text. `$unit` adds a PUEL unit.
`$schema` declares a reusable structure; `: schema_name` applies it to a
group, map item, or list item. A top-level `$override` replaces a value while
preserving its declared type and constraints.

```dipl {.verify}
$schema material
  density float kg/m3
materials[steel] : material
  density = 7850
$override
  materials[steel].density = 7800
```

## Project and tools

A `DIPfile` registers code, units, schemas, sources, and overrides. Project
paths resolve relative to that manifest; included DIPfiles may be nested.

```dipl {.illustrative}
code[]
  file = "parameters.dip"
overrides[]
  file = "run-settings.dip"
```

```bash {.illustrative}
snt dip validate --input file parameters.dip
snt dip parse --input file parameters.dip --print
snt puq eval "23*cm + 3*m"
snt puq convert "2.5*m" "cm"
snt report --project DIPfile --format pdf --output report.pdf
```

**C++:** `DIP::add_file()` / `add_project()`, then `parse()`.
**Python:** `DIP().add_string(...)` or `add_project(...)`, then `parse()`;
`Quantity("2.5*m")` parses PUEL. PDF reports may append this packaged
reference when requested by the application developer.

## Further reading

[DIPL specification](https://scinumtools.github.io/snt3/dipl/) ·
[PUEL specification](https://scinumtools.github.io/snt3/puel/).  
Project: [https://github.com/scinumtools/snt3](https://github.com/scinumtools/snt3)  
Docs: [https://scinumtools.github.io/snt3/](https://scinumtools.github.io/snt3/)
