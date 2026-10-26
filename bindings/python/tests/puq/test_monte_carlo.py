import math

import pytest

from scinumtools3.puq import Quantity


def test_nonlinear_square_and_uniform_input():
    x = Quantity(0.5, 0.4, "m")
    ordinary = x * x
    assert ordinary.value() == pytest.approx(0.25)
    assert ordinary.uncertainty() == pytest.approx(0.4)
    result = Quantity.monte_carlo(lambda x: x * x, [x], samples=100_000, seed=42)
    assert result.quantity.value() == pytest.approx(0.41, abs=0.01)
    assert result.quantity.uncertainty() == pytest.approx(math.sqrt(0.2112), abs=0.01)
    assert result.lower >= 0
    assert result.upper > 1.5
    assert (x * x).uncertainty() == pytest.approx(0.4)

    uniform = Quantity.monte_carlo(
        lambda x: x, [Quantity(10.0, 0.2, "m")],
        distributions=["uniform"], samples=100_000, seed=42,
    )
    assert uniform.quantity.value() == pytest.approx(10.0, abs=0.005)
    assert uniform.quantity.uncertainty() == pytest.approx(0.2, abs=0.005)
    assert uniform.lower == pytest.approx(10 - 0.95 * math.sqrt(3) * 0.2, abs=0.005)
    assert uniform.upper == pytest.approx(10 + 0.95 * math.sqrt(3) * 0.2, abs=0.005)


def test_sources_are_reused_and_invalid_samples_fail():
    x = Quantity(2.0, 0.3, "m")
    cancelled = Quantity.monte_carlo(
        lambda left, right: left - right, [x, x], samples=1000, seed=42,
    )
    assert cancelled.quantity.uncertainty() == 0.0
