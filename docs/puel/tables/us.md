# US: United States customary units

Select this system with the leading `US_` prefix. The [shared base-unit symbols](common.md#base-unit-symbols) are also available. Definitions below are interpreted within this system; aliases and capitalization are significant. Prefix rules follow the [shared table legend](common.md).

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
| `p` | point | `352.778e-6*m` | None |
| `P` | pica | `4.233e-3*m` | None |
| `sqft` | sq. survey foot | `0.09290341*m2` | None |
| `sqch` | square chain | `404.68564224*m2` | None |
| `section` | section | `2.589998e6*m2` | None |
| `twp` | survey township | `93.23993e6*m2` | None |
| `cuin` | cubic inch | `16.387064e-6*m3` | None |
| `cuft` | cubic foot | `28.316846592e-3*m3` | None |
| `cuyd` | cubic yard | `0.764554857984*m3` | None |
| `acft` | acre-foot | `1233.482*m3` | None |
| `minim` | minim | `61.611519921875e-9*m3` | None |
| `fldr` | fluid drachm | `3.6966911953125e-6*m3` | None |
| `tsp` | teaspoon | `4.92892159375e-6*m3` | None |
| `tbsp` | tablespoon | `14.78676478125e-6*m3` | None |
| `floz` | fluid ounce | `29.5735295625e-6*m3` | None |
| `jig` | jig | `44.36029434375e-6*m3` | None |
| `gi` | gill | `118.29411825e-6*m3` | None |
| `c` | cup | `236.5882365e-6*m3` | None |
| `pt` | pint | `0.473176473e-3*m3` | None |
| `qt` | quart | `0.946352946e-3*m3` | None |
| `pot` | pottle | `1.892705892e-3*m3` | None |
| `gal` | gallon | `3.785411784e-3*m3` | None |
| `bbl` | liquid barrel | `119.240471196e-3*m3` | None |
| `bblo` | oil barrel | `158.987294928e-3*m3` | None |
| `hhd` | hogshead | `238.480942392e-3*m3` | None |
| `ptd` | dry pint | `0.55061047135749e-3*m3` | None |
| `qtd` | dry quart | `1.1012209427149e-3*m3` | None |
| `gald` | dry gallon | `4.4048837708599e-3*m3` | None |
| `pk` | peck | `8.8097675417199e-3*m3` | None |
| `bu` | bushel | `35.239070166879e-3*m3` | None |
| `bbll` | liquid barrel | `115.62712358400e-3*m3` | None |
| `cwts` | short hundredwe. | `45.359237e3*g` | None |
| `tons` | short ton | `907.18474e3*g` | None |
| `dwt` | pennyweight | `1.55517384*g` | None |
| `ozt` | troy ounce | `31.1034768*g` | None |
| `lbt` | troy pound | `373.241721*g` | None |

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
