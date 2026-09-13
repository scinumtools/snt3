# GU: Gaussian CGS units

Select this system with the leading `GU_` prefix. The [shared base-unit symbols](common.md#base-unit-symbols) are also available. Definitions below are interpreted within this system; aliases and capitalization are significant. Prefix rules follow the [shared table legend](common.md).

## Derived units and aliases

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `Gal` | Gal | `cm/s2` | None |
| `dyn` | dyne | `g*cm/s2` | All |
| `Ba` | Barye | `dyn/cm2` | None |
| `b` | Barye | `Ba` | None |
| `erg` | erg | `dyn*cm` | None |
| `St` | Stokes | `cm2/s` | `c` |
| `P` | Poise | `g/(cm*s)` | `c` |
| `Ka` | Kayser | `cm-1` | None |
| `Pma` | permeability | `1e-4*m2` | None |
| `Rayl` | Rayl | `g/(cm2*s)` | None |
| `radd` | radiation dose | `100*erg/g` | None |
| `Fr` | franklin | `dyn1:2*cm` | None |
| `statC` | Statcoulomb | `Fr` | None |
| `esu` | elstat. u.o. charge | `Fr` | None |
| `statA` | Statampere | `Fr/s` | None |
| `statV` | Statvolt | `erg/Fr` | None |
| `statOhm` | Statohm | `s/cm` | None |
| `statF` | Statfarad | `cm` | None |
| `statH` | Stathenry | `s2/cm` | None |
| `G` | Gauss | `cm-1:2*g1:2*s-1` | None |
| `Oe` | Oersted | `g1:2/(cm1:2*s)` | None |
| `Gb` | Gilbert | `Oe*cm` | None |
| `Mx` | Maxwell | `dyn1:2*cm2` | None |

## System-specific constants

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `{m_u}` | atom. mass const. | `1.660539069e-24*g` | None |
| `{a_0}` | Bohr radius | `5.291772105e-9*cm` | None |
| `{k}` | Boltzmann const. | `1.380649e-16*erg/K` | None |
| `{m_e}` | electron mass | `9.10938371e-28*g` | None |
| `{alpha}` | fine-str. const. | `0.007297352564` | None |
| `{G}` | gravit. const. | `6.6743e-8*dyn*cm2/g2` | None |
| `{h}` | Planck const. | `6.62607015e-27*erg*s` | None |
| `{hbar}` | reduced Pl. con. | `1.054571817e-27*erg*s` | None |
| `{c}` | speed of light | `2.99792458e10*cm/s` | None |
| `{pi}` | pi num. | `3.1415926` | None |
| `{e}` | elementary charge | `4.80320471e-10*Fr` | None |
| `{mu_B}` | Bohr magneton | `9.274010066e-21*erg/G` | None |

## Quantity symbols and conversion factors

`<name>` denotes the quantity scale in the third column. Where provided, the last column defines `|name|`, the factor used by contextual conversion to relate that scale to SI. A dash means no explicit factor is defined. The factor expressions are evaluated using SI metre-kilogram-second scaling, and their numerical results are stored as dimensionless conversion coefficients.

| Symbol | Meaning | Quantity scale | Conversion factor |
| --- | --- | --- | --- |
| `<l>` | length | `cm` | — |
| `<m>` | mass | `g` | — |
| `<t>` | time | `s` | — |
| `<Iv>` | luminous intensity | `cd` | — |
| `<n>` | amount of substance | `mol` | — |
| `<the>` | angular displacement | `rad` | — |
| `<v>` | velocity | `cm/s` | — |
| `<a>` | acceleration | `Gal` | — |
| `<F>` | force | `dyn` | — |
| `<E>` | energy | `erg` | — |
| `<P>` | power | `erg/s` | — |
| `<p>` | pressure | `Ba` | — |
| `<mu_v>` | dynamic viscosity | `P` | — |
| `<nu_v>` | kinematic viscosity | `St` | — |
| `<k_n>` | wave number | `Ka` | — |
| `<q>` | electric charge | `statC` | `1e1/({c}*100)` |
| `<I>` | current | `statA` | `1e1/({c}*100)` |
| `<phi_e>` | electric potential | `statV` | `1e-8*({c}*100)` |
| `<E_e>` | electric field strength | `statV/cm` | `1e-6*({c}*100)` |
| `<D_e>` | electric displacement field | `statC/cm2` | `1e5/(4*{pi}*({c}*100))` |
| `<p_e>` | electric dipole moment | `statC*cm` | `1e-1/({c}*100)` |
| `<Phi_E>` | electric flux | `statC` | `1e1/(4*{pi}*({c}*100))` |
| `<eps>` | permittivity | `1` | `1e11/(4*{pi}*({c}*100)2)` |
| `<R>` | electrical resistance | `statOhm` | `({c}*100)2/1e9` |
| `<rho_e>` | electrical resistivity | `statOhm*cm` | `({c}*100)2/1e11` |
| `<C>` | capacitance | `statF` | `1e9/({c}*100)2` |
| `<L>` | inductance | `statH` | `({c}*100)2/1e9` |
| `<B>` | magnetic flux density | `G` | `1e-4` |
| `<H>` | magnetic field strenght | `Oe` | `1e3/(4*{pi})` |
| `<mm>` | magnetic dipole moment | `erg/G` | `1e-3` |
| `<Phi_M>` | magnetic flux | `Mx` | `1e-8` |
| `<mu>` | permeability | `1` | `4*{pi}*1e-7` |
