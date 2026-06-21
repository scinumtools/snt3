import sys
from pathlib import Path
import pytest

repo = Path(__file__).resolve().parents[3]

sys.path.insert(0, str(repo / "build/python"))

print("\nFIRST 10 sys.path entries:")
for p in sys.path[:10]:
    print(" ", p)

import importlib.util

spec = importlib.util.find_spec("scinumtools3")
print("\nscinumtools3 spec =", spec)

if spec:
    print("origin =", spec.origin)
    print("locations =", spec.submodule_search_locations)

@pytest.fixture(scope="session")
def data_dir():
    return Path(__file__).parent
