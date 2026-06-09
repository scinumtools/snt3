from .._core import api as _api

import sys

sys.modules[__name__ + ".dip"] = _api.dip
sys.modules[__name__ + ".puq"] = _api.puq
