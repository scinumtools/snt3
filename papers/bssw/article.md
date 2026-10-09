# Keeping Scientific Input Rules Close to the Parameters

A scientific code may read a YAML file long before it reaches the numerical
solver. The file contains values, but the application must decide what those
values mean: whether a number is a length or a time, whether a count fits its
expected integer type, which settings can be overridden, and how derived
values change when an input changes.

This work is necessary. It can also spread a parameter's contract across the
input file and several functions in the host language. That makes a run harder
to inspect and a solver harder to reuse. A small heat-diffusion example shows
what changes when the contract lives in an evaluated parameter model.

## One Solver, Two Input Paths

The companion example models heat spreading along a 10 cm rod. Its ends remain
at 320 K, while the interior starts at 280 K. It runs two material cases: a
baseline thermal diffusivity of 1 mm²/s and a trial override of 0.02 cm²/s,
equivalent to 2 mm²/s. Both cases use the same timestep and the same Python
heat-update function.

The files separate scientific work from input handling:

```text
example/
  core.py              # shared heat-update loop and output
  run_yaml.py          # YAML and Pint input handling
  run_dipl.py          # DIPL input handling
  run.py               # evaluate the two paths in sequence
  parameters.yaml
  parameters.dip
```

The entry point compares the two outputs. For the supplied inputs, both paths
produce a baseline center temperature of 281.948 K, a trial center
temperature of 288.851 K, and a timestep of 20 seconds. This isolates the
comparison: the input systems differ; the numerical calculation does not.

The [companion example](https://github.com/scinumtools/snt3/tree/main/papers/bssw/example)
is in the SciNumTools3 repository.

From the repository root, run `cd papers/bssw/example && ./setup.sh`. This
creates a Python virtual environment, installs the dependencies, and runs both
input paths in sequence.

## YAML and Pint: Capable Tools, Application-Owned Rules

YAML stores the two material cases. An anchor reuses the baseline data for the
trial, and an `overrides` mapping records the trial's change:

```yaml
materials:
  baseline: &thermal_material
    diffusivity:
      value: 1
      unit: mm**2/s
  trial:
    <<: *thermal_material

overrides:
  materials.trial.diffusivity: {value: 0.02, unit: cm**2/s}
```

Pint handles physical units in the YAML runner. The runner tells Pint that
diffusivity must convert to `m**2/s`; Pint rejects an incompatible value such
as `2 s`. Python also applies the override, checks positive values and
integer bounds, validates the run ID with a regular expression, calculates
the timestep, and checks the diffusion factors for stability. The YAML anchor
reuses data; it does not validate a material schema. This example validates
the fields its runner uses, rather than every possible YAML field.

This approach works. It also means that understanding the parameter contract
requires reading both the YAML file and the runner. Another validation
library could shorten the runner, while adding a separate schema and its
integration with Pint and override handling.

## DIPL: An Evaluated Parameter Contract

The second path uses the Dimensional Input Parameter Language (DIPL), part of
SciNumTools3. Its C++ core evaluates the parameter model, which the example
accesses through Python bindings; the heat solver remains in Python. In the
DIPL input file, a reusable schema gives both material cases the same typed
diffusivity parameter, units, condition, and description:

```dipl
$schema thermal_material
  diffusivity float = 1 mm2/s
    !condition ({.} > 0 mm2/s)
    ?descr "Illustrative baseline, not measured material data"

materials map : thermal_material
materials[baseline]
materials[trial]

$override
  materials[trial].diffusivity = 0.02 cm2/s
```

The DIPL file also declares a `uint32` cell count, uses `!format` to restrict
the run ID, and derives the shared timestep and both diffusion factors through
parameter references. Conditions check that each factor is positive and at
most 0.5, the stability bound for this one-dimensional explicit update. As it
parses the model, DIPL applies the trial override and evaluates the derived
values using it; the override retains the target parameter's type, units, and
conditions. DIPL normalizes the override to approximately 2 mm²/s, the
declaration's unit, and the `run.alpha` expression converts that value to
2 × 10⁻⁶ m²/s. In the YAML path, Pint performs the conversion to m²/s when
Python requests the diffusivity. The DIPL runner passes evaluated values to
the same `core.py` solver used by the YAML path.

The distinction is visible in the runner files: `run_yaml.py` has 101 lines
of Python and `run_dipl.py` has 39. Both use the same 27-line numerical core.
Those counts describe this one implementation, not a lower bound for YAML or
a general productivity benchmark. They make a more useful point: the YAML
runner defines parameter behavior in Python, while the DIPL runner consumes a
model whose parameter behavior has already been evaluated.

## Testing Valid and Invalid Inputs

The matching temperatures check the normal run, but input failures are part
of the contract too. Replace the trial override with `2 s`: the YAML runner
asks Pint to convert seconds to area per time and stops before calling
`core.py`; DIPL rejects the override while evaluating the parameter model.
Change the run ID to `Heat Rod 01`, and the Python regular expression and
DIPL's `!format` both reject it. These cases exercise unit and format rules
alongside the positive-number constraints.

The unit-changing valid override is another useful test. The spelling
`0.02 cm²/s` differs from the declaration's `mm²/s`, yet both paths calculate
with the same physical diffusivity. The trial's higher diffusivity gives a
diffusion factor of 0.4, compared with 0.2 for the baseline, so its center
warms more in the twelve simulated steps. A run record should capture the
effective value and the input that produced it: two input
files can express the same quantity differently, and a deliberate override
can change all quantities derived from that input.

Here the initial temperature is one value in kelvin with a positive-value
condition, easy to check by hand. In a larger code, choosing initial
conditions or an initial-condition file may require knowing valid units,
ranges, and compatibility with other settings. Putting those rules in the
parameter model gives an AI agent inspectable guidance; with the YAML path
here, it would also need to recover rules from the runner. SNT can preview a
candidate and report rule violations and changed effective values without
editing the input file. That guidance covers only relationships the model
declares; a researcher still judges physical suitability.

## Information Beyond the Effective Value

Scientific inputs need context as well as valid numbers. The example marks
the diffusivity as illustrative rather than measured data. DIPL preserves
that description in the evaluated parameter's metadata after the override.
It also retains the source declaration and the applied override:

```python
trace = parameters["materials[trial].diffusivity"].provenance
print(trace.source_code.strip())
print(trace.override_code.strip())
```

Those calls report `diffusivity float = 1 mm2/s` and
`materials[trial].diffusivity = 0.02 cm2/s`; source line numbers are available
too. The YAML runner explicitly preserves its descriptive metadata when it
replaces a value. PyYAML's `safe_load()` does not include source locations in
the mappings it returns, so comparable source tracing would need additional
code. Neither format can invent a publication, measurement method, or DOI:
meaningful provenance still depends on information supplied by the author.

For a research workflow, the declaration text, override text, evaluated
value, and units can help explain a result after the run. They are only part
of a reproducible record. The solver version, dependency versions, numerical
method, and any data sources still need to be recorded by the surrounding
workflow. DIPL makes parameter history available to that workflow; it does
not create a complete experiment log on its own.

## When This Separation Helps

For a small code with a few stable settings, YAML plus Pint may be enough.
DIPL adds a dependency and a syntax that contributors must learn. It becomes
more attractive when a simulation has many related parameters, repeated
structures, unit-aware derived values, and runs that use explicit overrides.
The evaluated model can then serve the solver and other tools that inspect
values, units, constraints, and metadata.

Adoption can be incremental. Keep the numerical solver and its input
interface, declare one parameter group in DIPL, move its validation and
derived values into the parameter file, then compare normal and invalid runs
with the existing implementation. The paired example follows that pattern:
its `core.py` is shared, and only the input runners differ.

The general research software lesson is to make the parameter contract easy
to find and test. A solver should receive well-defined inputs, while the
choices that produced those inputs remain inspectable. The paired example
shows one way to build that boundary and makes its costs visible alongside
its benefits.
