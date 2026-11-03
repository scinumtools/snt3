#!/usr/bin/env bash
set -euo pipefail

# Resolve paths from the script, so it works when launched from another folder.
example_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${example_dir}/../../.." && pwd)"
snt="${SNT_EXE:-${repo_root}/build/bin/snt}"
python="${PYTHON:-python3}"
# A new directory keeps the input archive, experiment, and external artifact
# together. Experiment preparation refuses to overwrite an existing archive.
run_dir="${1:-$(mktemp -d "${TMPDIR:-/tmp}/snt-diph5-example.XXXXXX")}"
mkdir -p "$run_dir"
run_dir="$(cd "$run_dir" && pwd)"

environment="$run_dir/environment.diph5"
experiment="$run_dir/experiment.diph5"

# 1. Parse the project once and freeze its source text and evaluated values.
# Later stages read this archive, rather than parsing a possibly changed file.
"$snt" dip parse --project "$example_dir/DIPfile" --save "$environment"
# Extract scalar inputs from the saved environment for the external program.
steps="$("$snt" dip parse --load "$environment" --request experiment.steps --value --type integer)"
gain="$("$snt" dip parse --load "$environment" --request experiment.gain --value --type float)"

# 2. Make a separate experiment archive from that frozen environment.
# start records the command before launching it. Because this shell launches
# the process, we truthfully mark the invocation as host_reported.
"$snt" dip experiment prepare --input "$environment" --output "$experiment"
"$snt" dip experiment start --file "$experiment" --origin host_reported \
  --role scientific_program --cwd "$run_dir" -- \
  "$python" "$example_dir/simulate.py" "$steps" "$gain" "$run_dir"

# 3. Run exactly the argv recorded above. Results can be added while the
# experiment is running. The handoff files are written by simulate.py.
if "$python" "$example_dir/simulate.py" "$steps" "$gain" "$run_dir"; then
  # Embed the numeric samples with their shape and axis name.
  "$snt" dip experiment add --file "$experiment" --name trajectory \
    --values "$(cat "$run_dir/values.txt")" --shape "$steps" \
    --dimensions sample --description "Scaled sample values"
  # A diagnostic is a named observation, separate from the result array.
  "$snt" dip experiment diagnostic --file "$experiment" --name final_value \
    --values "$(cat "$run_dir/final_value.txt")" --category observed
  # Keep the CSV external, but record its location, size, and SHA-256.
  "$snt" dip experiment attach --file "$experiment" --name trajectory_csv \
    --artifact "$run_dir/trajectory.csv" --format CSV
  # Seal the inventory before marking the execution successful.
  "$snt" dip experiment finalize --file "$experiment"
  "$snt" dip experiment finish --file "$experiment" --status completed --exit-code 0
else
  # A failed program still leaves a useful experiment record and exit code.
  status=$?
  "$snt" dip experiment finish --file "$experiment" --status failed --exit-code "$status"
  exit "$status"
fi

# 4. Inspect the lifecycle and results, then check archive and artifact hashes.
"$snt" dip experiment show --file "$experiment"
"$snt" dip experiment results --file "$experiment"
"$snt" dip experiment verify --file "$experiment"
echo "Example output: $run_dir"
