#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
venv_python="$root_dir/.venv/bin/python"

if [[ ! -x "$venv_python" ]]; then
  "${PYTHON:-python3}" -m venv "$root_dir/.venv"
fi

"$venv_python" -m pip install --disable-pip-version-check -r "$root_dir/requirements.txt"

"$venv_python" "$root_dir/run.py"
