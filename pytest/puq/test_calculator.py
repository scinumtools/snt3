import pytest
import importlib.util
import glob
import numpy as np

from scinumtools3.puq import Quantity, Calculator

def test_calculator():

    q = Calculator("34*km + 25*m")
    assert q.to_string() == "34.025*km"
    
    q = Calculator("29*kg * 2*mg")
    assert q.to_string() == "58*kg*mg"
