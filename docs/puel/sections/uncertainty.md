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

PUQ propagates standard uncertainties to first order. For addition and
subtraction, it assumes the input uncertainties are independent and combines
them by root-sum-square:

```text
(x ± dx) ± (y ± dy) = (x ± y) ± sqrt(dx² + dy²)
```

For multiplication and division, PUQ uses first-order derivatives and combines
the independent contributions by root-sum-square. For example:

```text
u(xy) = sqrt((y·u(x))² + (x·u(y))²)
u(x/y) = sqrt((u(x)/y)² + (x·u(y)/y²)²)
```

The public PUQ mathematical functions use first-order derivative propagation
where applicable. For example, for a constant exponent `p`:

```text
y = x^p
dy = |p * x^(p - 1)| * dx
```

For a variable exponent, the independent contributions are combined by
root-sum-square. Logarithmic conversion and mathematical functions propagate
uncertainty through their corresponding transformation.

By default, inputs are independent. Copies and differentiable operations retain
source identity, so reusing one uncertain quantity preserves its correlation
(for example, `x - x` has zero uncertainty). Correlations can be assigned to
primitive uncertain quantities in Python. For a pair, use
`Quantity.correlate(x, y, rho)`, where `rho` is between -1 and 1. For a set,
use `Quantity.correlate_many(values, covariance_matrix)`. The matrix diagonal
must match the input uncertainty variances; the inputs must be scalar primitive
quantities. The matrix must be symmetric and positive semidefinite.

For legacy conservative bounds, select interval propagation explicitly:

```python
Quantity.set_propagation_mode("interval")
# Restore the default first-order model when desired:
Quantity.set_propagation_mode("standard")
```

The propagation mode is process-wide. Standard mode is first-order and does
not represent distribution shape or nonlinear higher-order effects.

