# SRU: Units for special relativity

Select this system with the leading `SRU_` prefix. The [shared base-unit symbols](common.md#base-unit-symbols) are also available. Definitions below are interpreted within this system; aliases and capitalization are significant. Prefix rules follow the [shared table legend](common.md).

The [fixed reference constants](common.md#fixed-reference-constants) are also available.

## Derived units and aliases

This system defines its characteristic scales through the quantity symbols below rather than additional named unit symbols.

## System-specific constants

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `{c}` | speed of light | `1` | None |

## Quantity symbols and conversion factors

`<name>` denotes the quantity scale in the third column. Where provided, the last column defines `|name|`, the factor used by contextual conversion to relate that scale to SI. A dash means no explicit factor is defined. The factor expressions are evaluated using SI metre-kilogram-second scaling, and their numerical results are stored as dimensionless conversion coefficients.

| Symbol | Meaning | Quantity scale | Conversion factor |
| --- | --- | --- | --- |
| `<l>` | length | `m` | — |
| `<m>` | mass | `kg` | — |
| `<t>` | time | `m` | `{#c}-1` |
| `<E>` | energy | `kg` | `{#c}2` |
| `<p_mom>` | momentum | `kg` | `{#c}` |
| `<v>` | velocity | `1` | `{#c}` |
| `<L_ome>` | angular momentum | `kg*m` | `{#c}` |
| `<F>` | force | `kg/m` | `{#c}2` |
| `<a>` | acceleration | `m-1` | `{#c}2` |
| `<rho_E>` | energy density | `kg/m3` | `{#c}2` |
