import pytest

from scinumtools3.dip import DIP, Environment

def test_init():

    # initialize dip
    dip = DIP()
    dip.add_string("foo int = 3");
    env = dip.parse();
