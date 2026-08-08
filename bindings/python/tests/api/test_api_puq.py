import pytest

from scinumtools3.api.puq import PUQConvert, PUQInfo, PUQList, PUQEval

def test_convert():

    c = PUQConvert("cm", "km")
    
    assert c.execute() == "1e-5*km"

    c = PUQConvert("15*ft", "m")
    c.argument_input_system("US")
    c.argument_output_system("SI")
    c.argument_output_quantity("l")

    assert c.execute() == "4.572*m"
    
def test_eval():

    c = PUQEval("1*cm + 2*dm")

    assert c.execute() == "21*cm"

    c.argument_input_system("SI")
    c.argument_output_system("US")
    c.argument_output_units("ft")
    c.argument_output_quantity("l")

    assert c.execute() == "0.688976*ft"
    
def test_info():

    c = PUQInfo("J")

    assert c.execute() == """\nExpression:  J\n\nUnit system: SI (International System of Units)\nResult:   1\nBase units:  J\n\nQuantities:  E, Q, W, tau\n\nDimensions:\n\nBase   Num*Mag                   Numerical                 Physical                  \n------ ------------------------- ------------------------- ------------------------- \nMGS    1e3                       1e3                       m2*g*s-2                  \nMKS    1                         1                         m2*kg*s-2                 \nCGS    1e7                       1e7                       cm2*g*s-2                 \n\nBase units:\n\nPrefix   Symbol   Exponent   Name                           Definition                     Dimensions MGS         Allowed prefixes       \n-------- -------- ---------- ------------------------------ ------------------------------ ---------------------- ---------------------- \n         J        1          Joule                          N*m                            1e3*m2*g*s-2           all                    \n"""

    c = PUQInfo("ft2*lb")
    c.argument_input_system("US")

    assert c.execute() == """\nExpression:  ft2*lb\n\nUnit system: US (United States customary units)\nResult:   1\nBase units:  ft2*lb\n\nDimensions:\n\nBase   Num*Mag                   Numerical                 Physical                  \n------ ------------------------- ------------------------- ------------------------- \nMGS    42.1401                   42.1401                   m2*g                      \nMKS    0.0421401                 0.0421401                 m2*kg                     \nCGS    4.21401e5                 4.21401e5                 cm2*g                     \n\nBase units:\n\nPrefix   Symbol   Exponent   Name                           Definition                     Dimensions MGS         Allowed prefixes       \n-------- -------- ---------- ------------------------------ ------------------------------ ---------------------- ---------------------- \n         ft       2          foot                           0.3048*m                       0.092903*m2                                   \n         lb       1          pound                          453.59237*g                    453.592*g                                     \n"""
    
def test_list():

    c = PUQList("AU")
    
    assert c.execute() == """\nList 'AU' does not exist.\n\nAvailable lists:\n\nprefix  unit prefixes\nbase    base units\nderiv   derived units\nlog     logarithmic units\ntemp    temperature units\nconst   constants\nquant   quantities\nsys     unit systems\n"""

    c = PUQList("deriv")
    c.argument_system("AU")

    assert c.execute() == """Symbol    Name                   Result        Dimension                      Definition                Allowed prefixes       \n--------- ---------------------- ------------- ------------------------------ ------------------------- ---------------------- \nE_h       hartree                4.3597447276(40)e-15 m2*g*s-2                       {#hbar}2/({#m_e}*{a_0}2)                         \n"""
