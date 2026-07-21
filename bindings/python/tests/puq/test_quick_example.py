import pytest

from scinumtools3.puq import Quantity

def test_convert_quantity():

    length = Quantity(3.048, 'm')
    length = length.convert('US_ft')
    assert length.to_string() == "10*ft"
