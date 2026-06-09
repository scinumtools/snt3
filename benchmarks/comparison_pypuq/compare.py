import timeit
import time
import numpy as np

# ==========================================
# 1. SETUP CODES (Executed once per library)
# ==========================================
setup_snt = """
from scinumtools3.puq import Quantity
from scinumtools3.puq.systems import SystemType
"""

setup_pint = """
import pint
ureg = pint.UnitRegistry()
"""

setup_unyt = """
import unyt as u
"""

setup_astropy = """
import astropy.units as u
from astropy.units import imperial
"""

setup_sympy = """
from sympy.physics.units import meter, foot, inch, convert_to
"""

# ==========================================
# 2. SCALAR OPERATIONS (Math + Conversion)
# ==========================================
test_snt_scalar = """
q1 = Quantity(10,'m')
q2 = Quantity(5,'US_ft')
res = (q1 + q2).convert('US_in')
"""

test_pint_scalar = """
q1 = 10 * ureg.meter
q2 = 5 * ureg.foot
res = (q1 + q2).to(ureg.inch)
"""

test_unyt_scalar = """
q1 = 10 * u.meter
q2 = 5 * u.foot
res = (q1 + q2).to(u.inch)
"""

test_astropy_scalar = """
q1 = 10 * u.meter
q2 = 5 * imperial.foot
res = (q1 + q2).to(imperial.inch)
"""

test_sympy_scalar = """
q1 = 10 * meter
q2 = 5 * foot
res = convert_to(q1 + q2, inch)

# ==========================================
# 3. RUNNING SCALAR BENCHMARKS
# ==========================================
SCALAR_LOOPS = 5_000

print(f"--- Running Scalar Benchmarks ({SCALAR_LOOPS:,} iterations) ---")

t_snt = timeit.timeit(stmt=test_snt_scalar, setup=setup_snt, number=SCALAR_LOOPS)
print(f"snt:     {t_snt:.4f} seconds")

t_unyt = timeit.timeit(stmt=test_unyt_scalar, setup=setup_unyt, number=SCALAR_LOOPS)
print(f"unyt:    {t_unyt:.4f} seconds")

t_pint = timeit.timeit(stmt=test_pint_scalar, setup=setup_pint, number=SCALAR_LOOPS)
print(f"pint:    {t_pint:.4f} seconds")

t_astropy = timeit.timeit(stmt=test_astropy_scalar, setup=setup_astropy, number=SCALAR_LOOPS)
print(f"astropy: {t_astropy:.4f} seconds")

t_sympy = timeit.timeit(stmt=test_sympy_scalar, setup=setup_sympy, number=SCALAR_LOOPS)
print(f"sympy:   {t_sympy:.4f} seconds (Symbolic engine overhead)")


# ==========================================
# 4. ARRAY OPERATIONS BENCHMARK (Macro performance)
# ==========================================
print("\n--- Running Array Benchmarks (100,000 Elements) ---")
ARRAY_SIZE = 100_000
raw_array = np.random.rand(ARRAY_SIZE)

# -- SNT Array Test
from scinumtools3.puq import Quantity                                                                     
start = time.perf_counter()
snt_arr = Quantity(raw_array,'m')
snt_res = snt_arr.convert('mm')
print(f"snt Array:    {time.perf_counter() - start:.5f} seconds")

# -- Pint Array Test --
import pint
ureg = pint.UnitRegistry()
start = time.perf_counter()
pint_arr = raw_array * ureg.meter
pint_res = pint_arr.to(ureg.millimeter)
print(f"pint Array:    {time.perf_counter() - start:.5f} seconds")

# -- Unyt Array Test --
import unyt as u
start = time.perf_counter()
unyt_arr = raw_array * u.meter
unyt_res = unyt_arr.to(u.millimeter)
print(f"unyt Array:    {time.perf_counter() - start:.5f} seconds")

# -- Astropy Array Test --
import astropy.units as au
start = time.perf_counter()
astropy_arr = raw_array * au.meter
astropy_res = astropy_arr.to(au.millimeter)
print(f"astropy Array: {time.perf_counter() - start:.5f} seconds")

print("\n(Note: SymPy is excluded from array testing because it lacks vectorization support.)")
