import pytest
import importlib.util
import sys
import os
import glob
import numpy as np

from scinumtools3.puq import Quantity, UnitSystem
from scinumtools3.puq.systems import US, ESU, EMU

def test_quantity():

    q = Quantity('23*km')          # default unit system is used
    assert q.unit_system() == "SI"
    q = Quantity('US_23*ft')       # unit system is defined in the expression
    assert q.unit_system() == "US"
    q = Quantity('23*ft', US)      # explicitely defined unit system
    assert q.unit_system() == "US"

def test_environment():
    
    with UnitSystem(EMU) as us:  # set unit system in a context manager
        assert Quantity('abA').unit_system() == "EMU"
    assert Quantity('km').unit_system() == "SI"
        
    us = UnitSystem(ESU)    # unit system environment is applied
    q = Quantity('statA')    
    assert q.unit_system() == "ESU"
    us.change(US)           # change of unit system environment
    q = Quantity('ft')    
    assert q.unit_system() == "US"
    us.close()              # unit system environment is closed
    q = Quantity('au')    
    assert q.unit_system() == "SI"
