# Unit systems

The following tables specify the built-in vocabulary provided by PUQ for PUEL expressions. A system identifier appears only at the beginning of an expression and is immediately followed by `_`, for example `SI_2*J` or `US_2*gal`. An identifier selects the entire expression; it cannot qualify an individual factor.

| Identifier | System | Unit and quantity tables |
| --- | --- | --- |
| `SI` | International System of Units | [SI definitions](si.md) |
| `ESU` | Electrostatic CGS units | [ESU definitions](esu.md) |
| `EMU` | Electromagnetic CGS units | [EMU definitions](emu.md) |
| `GU` | Gaussian CGS units | [GU definitions](gu.md) |
| `IU` | Imperial units | [IU definitions](iu.md) |
| `US` | United States customary units | [US definitions](us.md) |
| `AU` | Atomic units | [AU definitions](au.md) |
| `PU` | Planck units | [PU definitions](pu.md) |
| `SRU` | Units for special relativity | [SRU definitions](sru.md) |
| `GRU` | Units for general relativity | [GRU definitions](gru.md) |
| `GEO` | Geometrized units | [GEO definitions](geo.md) |

## Reading the tables

- Every system includes the [shared base-unit symbols](common.md#base-unit-symbols). Each system page lists its additional units and aliases, constants, and quantity scales. A unit available in SI is not automatically available in another system.
- Definitions describe one unprefixed unit in terms of symbols from the same system. Prefix permissions apply to the symbol in that row; prefixes are not implicitly inherited through an alias.
- Numerical factors and constant values reproduce the PUQ definitions, including their stated precision. These tables describe the PUEL vocabulary and do not redefine external metrological standards.
- Temperature and logarithmic rows give reference units, not linear equalities. Use the [special conversion rules](../specification.md#logarithmic-and-temperature-conversions); for example, `Cel` is not equal to `K`.
- SI is always available in PUQ. Other system families, offset temperature units, and logarithmic units depend on enabled implementation features.
- Natural systems often use quantity scales such as `<E>` rather than a new named unit. Constants without `#` may be normalized to unity; fixed `{#...}` reference constants retain their reference values.
