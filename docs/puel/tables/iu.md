# IU: Imperial units

Select this system with the leading `IU_` prefix. The [shared base-unit symbols](common.md#base-unit-symbols) are also available. Definitions below are interpreted within this system; aliases and capitalization are significant. Prefix rules follow the [shared table legend](common.md).

## Derived units and aliases

| Symbol | Meaning | Definition or reference | Prefixes |
| --- | --- | --- | --- |
| `twip` | twip | `17.6388888e-6*m` | None |
| `th` | thou | `25.4e-6*m` | None |
| `mil` | mil | `th` | None |
| `in` | inch | `0.0254*m` | None |
| `''` | inch | `in` | None |
| `li` | link | `0.201168*m` | None |
| `ft` | foot | `0.3048*m` | None |
| `'` | foot | `ft` | None |
| `yd` | yard | `0.9144*m` | None |
| `rd` | rod/pole/perch | `5.0292*m` | None |
| `ch` | chain | `20.1168*m` | None |
| `fur` | furlong | `201.168*m` | None |
| `mi` | mile | `1609.344*m` | None |
| `lea` | league | `4828.032*m` | None |
| `le` | league | `lea` | None |
| `ftm` | fathom | `1.8288*m` | None |
| `cb` | cable | `219.456*m` | None |
| `nmi` | nautical mile | `1852*m` | None |
| `NM` | nautical mile | `nmi` | None |
| `ac` | acre | `4046.8564224*m2` | None |
| `gr` | grain | `64.79891e-3*g` | None |
| `dr` | drachm/dram | `1.771845195*g` | None |
| `oz` | ounce | `28.349523125*g` | None |
| `lb` | pound | `453.59237*g` | None |
| `cwt` | long hundredweight | `50.80234544e3*g` | None |
| `ton` | long ton | `1.0160469088e6*g` | None |
| `slug` | slug | `14.59390294e3*g` | None |
| `min` | minute | `60*s` | None |
| `h` | hour | `60*min` | None |
| `day` | day | `24*h` | None |
| `yr_t` | tropical year | `365.24219*day` | `k`, `m`, `G` |
| `yr_j` | Julian year | `365.25*day` | `k`, `m`, `G` |
| `yr_g` | Gregorian year | `365.2425*day` | `k`, `m`, `G` |
| `yr` | year | `yr_j` | `k`, `m`, `G` |
| `degR` | degree Rankine | `5/9*K` | None |
| `mph` | miles per hour | `mi/h` | None |
| `lbf` | pound-force | `4.448222e3*g*m*s-2` | None |
| `Cel` | degree Celsius | `K` (temperature reference; offset conversion) | None |
| `degF` | degree Fahrenheit | `K` (temperature reference; offset conversion) | None |
| `bc` | barleycorn | `8.4667e-3*m` | None |
| `hh` | hand | `0.1016*m` | None |
| `sqrd` | sq. rod/po./per. | `25.29285264*m2` | None |
| `ro` | rood | `1011.7141056*m2` | None |
| `sqmi` | square mile | `2589988.110336*m2` | None |
| `minim` | minim | `59.1938802083e-9*m3` | None |
| `fls` | fluid scruple | `1.18387760416e-6*m3` | None |
| `fldr` | fluid drachm | `3.5516328125e-6*m3` | None |
| `floz` | fluid ounce | `28.4130625e-6*m3` | None |
| `gi` | gill | `142.0653125e-6*m3` | None |
| `pt` | pint | `568.26125e-6*m3` | None |
| `qt` | quart | `1136.5225e-6*m3` | None |
| `gal` | gallon | `4.54609e-3*m3` | None |
| `bblo` | oil barrel | `0.16365924*m3` | None |
| `st` | stone | `6.35029318e3*g` | None |
| `qr` | quarter | `12.70058636e3*g` | None |

## Quantity symbols and conversion factors

`<name>` denotes the quantity scale in the third column. Where provided, the last column defines `|name|`, the factor used by contextual conversion to relate that scale to SI. A dash means no explicit factor is defined. The factor expressions are evaluated using SI metre-kilogram-second scaling, and their numerical results are stored as dimensionless conversion coefficients.

| Symbol | Meaning | Quantity scale | Conversion factor |
| --- | --- | --- | --- |
| `<l>` | length | `ft` | — |
| `<m>` | mass | `lb` | — |
| `<t>` | time | `s` | — |
| `<Iv>` | luminous intensity | `cd` | — |
| `<n>` | amount of substance | `mol` | — |
| `<the>` | angular displacement | `rad` | — |
| `<V>` | volume | `gal` | — |
| `<A>` | area | `ac` | — |
| `<v>` | velocity | `mph` | — |
