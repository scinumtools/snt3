# PU: Planck units

Select this system with the leading `PU_` prefix. The [shared base-unit symbols](common.md#base-unit-symbols) are also available. Definitions below are interpreted within this system; aliases and capitalization are significant. Prefix rules follow the [shared table legend](common.md).

The [fixed reference constants](common.md#fixed-reference-constants) are also available.

## Derived units and aliases

This system defines its characteristic scales through the quantity symbols below rather than additional named unit symbols.

## System-specific constants

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `{c}` | speed of light | `1` | None |
| `{G}` | grav. const. | `1` | None |
| `{hbar}` | reduced Pl. con. | `1` | None |
| `{k_B}` | Boltzmann const. | `1` | None |

## Quantity symbols and conversion factors

`<name>` denotes the quantity scale in the third column. Where provided, the last column defines `|name|`, the factor used by contextual conversion to relate that scale to SI. A dash means no explicit factor is defined. The factor expressions are evaluated using SI metre-kilogram-second scaling, and their numerical results are stored as dimensionless conversion coefficients.

| Symbol | Meaning | Quantity scale | Conversion factor |
| --- | --- | --- | --- |
| `<l>` | length | `({#hbar}*{#G}/{#c}3)1:2` | — |
| `<m>` | mass | `({#hbar}*{#c}/{#G})1:2` | — |
| `<t>` | time | `({#hbar}*{#G}/{#c}5)1:2` | — |
| `<T>` | temperature | `({#hbar}*{#c}5*{k_B}-2/{#G})1:2` | — |
| `<A>` | area | `{#hbar}*{#G}/{#c}3` | — |
| `<V>` | volume | `({#hbar}*{#G}*{#c}-3)3:2` | — |
| `<p_mom>` | momentum | `({#hbar}*{#c}3/{#G})1:2` | — |
| `<E>` | energy | `({#hbar}*{#c}5/{#G})1:2` | — |
| `<F>` | force | `{#c}4/{#G}` | — |
| `<rho>` | density | `{#c}5/{#hbar}*{#G}-2` | — |
| `<a>` | acceleration | `({#c}7/{#hbar}/{#G})1:2` | — |
