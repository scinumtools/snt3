# Uncertainty Propagation

A scalar PUEL uncertainty is written immediately after its estimate, for
example `3.452(3)*kg`. The parenthesized digits represent an **absolute**
uncertainty in the last significant digits of the estimate. Thus
`3.452(3)` represents `3.452 ± 0.003`, while `3.452(3)e3` represents
`3452 ± 3`.

Array literals may attach an uncertainty to each element:

```PUEL
[2.00(20),3.00(30)]*m
```

The estimate and uncertainty arrays MUST have the same shape. A quantity
without parenthesized uncertainty has no stored uncertainty.

## Reference PUQ propagation model

PUQ stores absolute uncertainty in the same units as the estimate. Linear unit
conversion scales both estimate and uncertainty by the conversion factor.
Offset temperature conversion changes the estimate by its offset while scaling
the uncertainty only by the scale.

For addition and subtraction, the reference implementation uses a conservative
sum of absolute uncertainties:

```text
(x ± dx) ± (y ± dy) = (x ± y) ± (dx + dy)
```

For multiplication and division, it evaluates the extrema formed by the input
intervals and retains the larger absolute deviation from the central result.
This is an interval-style bound, not Gaussian root-sum-square propagation.

The public PUQ mathematical functions use first-order derivative propagation
where applicable. For example, for a constant exponent `p`:

```text
y = x^p
dy = |p * x^(p - 1)| * dx
```

For a variable exponent, the independent contributions are combined by
root-sum-square. Logarithmic conversion and mathematical functions propagate
uncertainty through their corresponding transformation.

PUQ does not model correlations between input uncertainties. Applications
requiring covariance-aware or statistical uncertainty analysis MUST provide
that model outside the PUEL expression language.

