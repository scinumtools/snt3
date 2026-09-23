# Special Units and Conformance Options

Most PUEL units are linear: they convert through a scale factor and can be
freely combined by unit algebra. Temperature and logarithmic units are
different. They are valid PUEL symbols, but conversion requires specialized
semantics.

## Temperature units

`K`, `Cel`, `degF`, and `degR` represent absolute temperature scales. A
conversion may contain both a scale and an offset:

```text
23*Cel = 296.15*K
```

Consequently, `Cel` is not a multiplicative synonym for `K`. When a temperature
factor occurs with one ordinary linear factor, PUQ can apply its specialized
conversion to expressions such as `Cel/m`. Rebase-to-base-dimensions is not
defined for expressions containing temperature units.

## Logarithmic units

Logarithmic ratios and levels include `AR`, `PR`, `B`, `dB`, `Np`, `dBm`,
`dBW`, and the reference levels listed in the unit-system tables. A ratio has
no absolute reference; a level does. For example:

```text
0*dBm = 1*mW
0*dBW = 1*W
```

The complete tables define each level's reference quantity. PUQ supports a
single logarithmic factor with an optional ordinary linear factor, for example
`dBmW/Hz`. It does not define arbitrary algebraic rebasing of logarithmic
units; rebase-to-base-dimensions is invalid for expressions containing one.

## Conversion availability

Matching physical dimensions are necessary but not sufficient for a special
conversion. The source and target special-unit combination must have a defined
conversion rule. Otherwise a converter MUST report that the conversion is
unavailable rather than applying a linear scale factor.

For conversions between systems whose dimensional conventions differ, a
physical-quantity context may also be required. For example, `I` identifies
electric current when converting between `statA` and `A`; energy conversion
between `erg` and `J` does not require that context.

## Reference implementation feature matrix

The PUQ reference implementation always provides SI. Its compile-time
configuration can enable or disable the following optional feature families:

| Feature family | PUEL vocabulary affected |
| --- | --- |
| CGS systems | `ESU`, `EMU`, and `GU` units and quantity scales |
| English systems | `IU` and `US` units |
| Natural systems | `AU`, `PU`, `SRU`, `GRU`, and `GEO` units and quantity scales |
| Temperature units | `Cel`, `degF`, and `degR` with offset conversions |
| Logarithmic units | ratios, bel/neper units, and referenced logarithmic levels |
| Unicode preprocessing | display-oriented Unicode input aliases; portable PUEL remains ASCII |

The SciNumTools3 default configuration enables these feature families. A
consumer that exchanges PUEL expressions across implementations SHOULD either
use the portable SI linear subset or record the required systems and special
unit features with the data.

