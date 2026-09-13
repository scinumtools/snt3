# GRU: Units for general relativity

Select this system with the leading `GRU_` prefix. The [shared base-unit symbols](common.md#base-unit-symbols) are also available. Definitions below are interpreted within this system; aliases and capitalization are significant. Prefix rules follow the [shared table legend](common.md).

The [fixed reference constants](common.md#fixed-reference-constants) are also available.

## Derived units and aliases

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `eV` | electronvolt | `1.602176634e-19*m2*kg*s-2` | `k`, `M`, `G`, `T` |

## System-specific constants

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `{c}` | speed of light | `1` | None |
| `{hbar}` | reduced Pl. con. | `1` | None |
| `{eps_0}` | permit. of vac. | `1` | None |
| `{k_B}` | Boltzmann const. | `1` | None |
| `{pi}` | pi num. | `{#pi}` | None |
| `{alpha}` | fine-str. const. | `{#e}2/(4*{pi}*{#eps_0}*{#hbar}*{#c})` | None |
| `{e}` | elem. charge | `(4*{pi}*{alpha})1:2` | None |

## Quantity symbols and conversion factors

`<name>` denotes the quantity scale in the third column. Where provided, the last column defines `|name|`, the factor used by contextual conversion to relate that scale to SI. A dash means no explicit factor is defined. The factor expressions are evaluated using SI metre-kilogram-second scaling, and their numerical results are stored as dimensionless conversion coefficients.

| Symbol | Meaning | Quantity scale | Conversion factor |
| --- | --- | --- | --- |
| `<l>` | length | `GeV-1` | `{#hbar}*{#c}` |
| `<m>` | mass | `GeV` | `{#c}-2` |
| `<t>` | time | `GeV-1` | `{#hbar}` |
| `<E>` | energy | `GeV` | — |
| `<p_mom>` | momentum | `GeV` | `{#c}-1` |
| `<v>` | velocity | `1` | `{#c}` |
| `<L_ome>` | angular momentum | `1` | `{#hbar}` |
| `<A>` | area | `GeV-2` | `({#hbar}*{#c})2` |
| `<F>` | force | `GeV2` | `({#hbar}*{#c})-1` |
| `<rho_E>` | energy density | `GeV4` | `({#hbar}*{#c})-3` |
| `<q>` | electric charge | `{#e}/{e}` | — |
