import pytest

from scinumtools3.api.puq import PUQConvert, PUQInfo, PUQList, PUQEval

def test_convert():

    c = PUQConvert("cm", "km")
    
    # TODO: test arguments

    assert c.execute() == "1e-5*km"

def test_eval():

    c = PUQEval("1*cm + 2*dm")

    # TODO: test arguments

    assert c.execute() == "21*cm"

def test_info():

    c = PUQInfo("J")

    # TODO: test arguments
    
    assert c.execute() == """\nExpression:  J\n\nUnit system: SI (International System of Units)\nResult:   1\nBase units:  J\n\nQuantities:  E, Q, W, tau\n\nDimensions:\n\nBase   Num*Mag                   Numerical                 Physical                  \n------ ------------------------- ------------------------- ------------------------- \nMGS    1e3                       1e3                       m2*g*s-2                  \nMKS    1                         1                         m2*kg*s-2                 \nCGS    1e7                       1e7                       cm2*g*s-2                 \n\nBase units:\n\nPrefix   Symbol   Exponent   Name                           Definition                     Dimensions MGS         Allowed prefixes       \n-------- -------- ---------- ------------------------------ ------------------------------ ---------------------- ---------------------- \n         J        1          Joule                          N*m                            1e3*m2*g*s-2           all                    \n"""

def test_list():

    c = PUQList("AU")

    # TODO: test arguments
    
    assert c.execute() == """\nList 'AU' does not exist.\n\nAvailable lists:\n\nprefix  unit prefixes\nbase    base units\nderiv   derived units\nlog     logarithmic units\ntemp    temperature units\nconst   constants\nquant   quantities\nsys     unit systems\n"""
