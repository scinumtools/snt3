# AU: Atomic units

Select this system with the leading `AU_` prefix. The [shared base-unit symbols](common.md#base-unit-symbols) are also available. Definitions below are interpreted within this system; aliases and capitalization are significant. Prefix rules follow the [shared table legend](common.md).

The [fixed reference constants](common.md#fixed-reference-constants) are also available.

## Derived units and aliases

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `E_h` | hartree | `{#hbar}2/({#m_e}*{a_0}2)` | None |

## System-specific constants

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `{hbar}` | reduced Pl. con. | `1` | None |
| `{e}` | elementary charge | `1` | None |
| `{m_e}` | electr. rest mass | `1` | None |
| `{kap_0}` | permittivity | `1` | None |
| `{a_0}` | Bohr radius | `{#kap_0}*{#hbar}2/({#m_e}*{#e}2)` | None |
| `{alpha}` | fine-str. const. | `1/137` | None |
| `{c}` | speed of light | `{alpha}-1*{a_0}*E_h/{#hbar}` | None |
| `{r_e}` | class. e. radius | `{alpha}2*{a_0}` | None |
| `{lbar}` | red. Com. wavele. | `{alpha}*{a_0}` | None |
| `{m_p}` | proton mass | `1836*{#m_e}` | None |

## Quantity symbols and conversion factors

`<name>` denotes the quantity scale in the third column. Where provided, the last column defines `|name|`, the factor used by contextual conversion to relate that scale to SI. A dash means no explicit factor is defined. The factor expressions are evaluated using SI metre-kilogram-second scaling, and their numerical results are stored as dimensionless conversion coefficients.

| Symbol | Meaning | Quantity scale | Conversion factor |
| --- | --- | --- | --- |
| `<l>` | length | `{a_0}` | — |
| `<m>` | mass | `{#m_e}` | — |
| `<t>` | time | `{#hbar}/E_h` | — |
| `<I>` | current | `{#e}*E_h/{#hbar}` | — |
| `<Iv>` | luminous intensity | `cd` | — |
| `<n>` | amount of substance | `mol` | — |
| `<the>` | angular displacement | `rad` | — |
| `<v>` | velocity | `{a_0}*E_h/{#hbar}` | — |
| `<p_mom>` | momentum | `{#hbar}/{a_0}` | — |
| `<rho_q>` | electric charge density | `{#e}/{a_0}3` | — |
| `<q>` | electric charge | `{#e}` | — |
| `<p_e>` | electric dipole moment | `{#e}*{a_0}` | — |
| `<p_qe>` | electric quadrupole moment | `{#e}*{a_0}2` | — |
| `<phi_e>` | electric potential | `E_h/{#e}` | — |
| `<E_e>` | electric field strength | `E_h/{#e}*{a_0}` | — |
| `<V_e>` | electric field gradient | `E_h/{#e}*{a_0}2` | — |
| `<eps>` | permittivity | `{#kap_0}` | — |
| `<alp_e>` | electric polarizability | `{#e}2*{a_0}2/E_h` | — |
| `<alp_he>` | 1st hyperpolarizability | `{#e}3*{a_0}3/E_h2` | — |
| `<alp_hhe>` | 2nd hyperpolarizability | `{#e}4*{a_0}4/E_h3` | — |
| `<B>` | magnetic flux density | `{#hbar}/({#e}*{a_0}2)` | — |
| `<mm>` | magnetic dipole moment | `{#hbar}*{#e}/{#m_e}` | — |
| `<E>` | energy | `E_h` | — |
| `<F>` | force | `E_h/{a_0}` | — |
| `<S_a>` | action | `{#hbar}` | — |
