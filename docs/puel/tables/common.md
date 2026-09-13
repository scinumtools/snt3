# Shared unit definitions

These symbols are available in every listed system. Prefixes are case-sensitive: **All** means any prefix in the [prefix table](prefixes.md), **None** means no prefix, and an explicit list permits only those prefixes. An unprefixed symbol is always permitted.

## Base-unit symbols

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `m` | meter | `m` | All |
| `g` | gram | `g` | All |
| `s` | second | `s` | All |
| `K` | Kelvin | `K` | All |
| `A` | Ampere | `A` | All |
| `cd` | candela | `cd` | All |
| `mol` | mole | `mol` | All |
| `rad` | radian | `rad` | `m` |

## Fixed reference constants

The following constants are available in `SI`, `AU`, `PU`, `SRU`, `GRU`, and `GEO`. The `#` inside the braces distinguishes a fixed reference constant from a system-specific constant: for example, `{#c}` retains its reference value while `{c}` may be `1` in a natural system. Parenthesized digits denote uncertainty using the ordinary PUEL notation.

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `{#alpha}` | fine str. const. | `7.2973525643(11)e-3` | None |
| `{#euler}` | Euler's num. | `2.718281828459` | None |
| `{#N_0}` | Avogadro's num. | `6.02214076e23` | None |
| `{#pi}` | pi num. | `3.1415926535` | None |
| `{#a_0}` | Bohr radius | `5.29177210544(82)e-11*m` | None |
| `{#c}` | speed of light | `299792458*m/s` | None |
| `{#e}` | elem. charge | `1.602176634e-19*A*s` | None |
| `{#eps_0}` | permit. of vac. | `8.8541878188(14)e-12*m-3*kg-1*s4*A2` | None |
| `{#G}` | grav. const. | `6.67430(15)e-11*m3*kg-1*s-2` | None |
| `{#g}` | grav. accel. | `9.80665*m/s2` | None |
| `{#h}` | Planck const. | `6.62607015e-34*m2*kg*s-1` | None |
| `{#hbar}` | Reduced Pl. con. | `1.054571817e-34*m2*kg*s-1` | None |
| `{#k_B}` | Boltzmann const. | `1.380649e-23*m2*kg*s-2*K-1` | None |
| `{#mu_0}` | permeab. of vac. | `1.25663706127(20)e-6*m*kg*s-2*A-2` | None |
| `{#mu_B}` | Bohr magneton | `9.2740100657(29)e-24*m2*A` | None |
| `{#m_e}` | electron mass | `9.1093837139(28)e-31*kg` | None |
| `{#m_p}` | proton mass | `1.67262192595(52)e-27*kg` | None |
| `{#m_n}` | neutron mass | `1.67492749804(95)e-27*kg` | None |
| `{#R_inf}` | Rydberg constant | `10973731.568157(12)*m-1` | None |
| `{#sigma}` | Stef.Bolt. const. | `5.670374419e-8*kg*s-3*K-4` | None |
| `{#N_A}` | Avogadro's const. | `6.02214076e23/mol` | None |
| `{#kap_0}` | permittivity | `1.11265005620e-13*m-3*g-1*s4*A2` | None |
| `{#k_e}` | Coulomb const. | `8.9875517923(14)e9*m3*kg*s-4*A-2` | None |
| `{#H_0}` | Hubble const. | `67.36(54)*3.24076e-20*s-1` | None |
| `{#L_sol}` | Solar luminosity | `3.828e26*m2*kg*s-3` | None |
| `{#M_sol}` | Solar mass | `1.988475(92)e30*kg` | None |
| `{#R_sol}` | Solar radius | `6.9566(14)e8*m` | None |
