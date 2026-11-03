"""A tiny stand-in for a scientific program with a file result."""

import csv
from pathlib import Path
import sys


# These arguments are read from the saved DIPH5 configuration by run.sh.
# The simulator itself is deliberately independent of SNT and HDF5.
steps = int(sys.argv[1])
gain = float(sys.argv[2])
destination = Path(sys.argv[3])
values = [gain * (index + 1) for index in range(steps)]

# This is the program's actual output file. The experiment archive registers
# its path and hash; it does not embed the CSV bytes.
with (destination / "trajectory.csv").open("w", newline="") as stream:
    writer = csv.writer(stream)
    writer.writerow(("sample", "value"))
    writer.writerows((index + 1, value) for index, value in enumerate(values))

# These handoff files let the launcher register the same values as an embedded
# numeric array and a diagnostic. They are not themselves experiment results.
(destination / "values.txt").write_text(",".join(map(str, values)))
(destination / "final_value.txt").write_text(str(values[-1]))
