import numpy as np
import pytest

from scinumtools3.puq import Quantity, sqrt


def test_power_and_direct_sqrt_preserve_units_and_uncertainty():
    measured_square = Quantity(np.array([4.0, 9.0]), np.array([0.2, 0.3]), "m2")

    power_root = measured_square ** 0.5
    function_root = sqrt(measured_square)

    np.testing.assert_allclose(power_root.value(numpy=True), [2.0, 3.0])
    np.testing.assert_allclose(function_root.value(numpy=True), [2.0, 3.0])
    np.testing.assert_allclose(function_root.uncertainty(numpy=True), [0.05, 0.05])
    assert power_root.units == "m"
    assert function_root.units == "m"


def test_numpy_add_and_sqrt_keep_quantity_units():
    left = Quantity(np.array([1.0, 2.0, 3.0]), "m")
    right = Quantity(np.array([4.0, 5.0, 6.0]), "m")

    total = np.add(left, right)
    root = np.sqrt(Quantity(np.array([4.0, 9.0]), "m2"))

    np.testing.assert_allclose(total.value(numpy=True), [5.0, 7.0, 9.0])
    np.testing.assert_allclose(root.value(numpy=True), [2.0, 3.0])
    assert total.units == "m"
    assert root.units == "m"


def test_numpy_sum_reduces_all_values_and_uncertainties():
    plain = Quantity(np.array([1.0, 2.0, 3.0]), "km")
    plain_total = np.sum(plain, axis=None)
    assert plain_total.value() == 6.0
    assert plain_total.units == "km"

    measured = Quantity(
        np.array([1.0, 2.0, 3.0]),
        np.array([0.1, 0.2, 0.3]),
        "km",
    )

    total = np.sum(measured)

    assert total.value() == 6.0
    assert total.uncertainty() == pytest.approx(0.6)
    assert total.units == "km"
