#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./setup.sh [--python PATH] [-h|--help]

Create a Python environment, install dependencies, and compare both examples.
Requires Python 3.12 or later.

  --python PATH  Interpreter used when creating the environment.
  -h, --help     Show this help without installing or running anything.

PYTHON also selects the interpreter; --python takes precedence.
Otherwise, a supported python3, python3.14, python3.13, or python3.12 is selected.
An existing supported .venv is reused. An incompatible environment is backed
up before a replacement is created; its files are not deleted.
EOF
}

selected_python="${PYTHON:-}"
while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help) usage; exit 0 ;;
    --python)
      if [[ $# -lt 2 || -z "$2" || "$2" == -* ]]; then
        printf 'Error: --python requires an interpreter path or command.\n' >&2
        exit 2
      fi
      selected_python="$2"
      shift 2
      ;;
    *) printf 'Error: unknown argument: %s\nUse --help for usage.\n' "$1" >&2; exit 2 ;;
  esac
done

root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
venv_python="$root_dir/.venv/bin/python"

supported_python() {
  "$1" -c 'import sys; sys.exit(0 if sys.version_info >= (3, 12) else 1)' 2>/dev/null
}

if [[ -n "$selected_python" ]] && ! supported_python "$selected_python"; then
  printf 'Error: %s must be an available Python 3.12 or later interpreter.\n' "$selected_python" >&2
  exit 1
fi

if ! supported_python "$venv_python"; then
  if [[ -z "$selected_python" ]]; then
    for candidate in python3 python3.14 python3.13 python3.12; do
      if supported_python "$candidate"; then
        selected_python="$candidate"
        break
      fi
    done
  fi
  if [[ -z "$selected_python" ]]; then
    printf 'Error: Python 3.12 or later is required.\nInstall it, then run ./setup.sh --python python3.12\n' >&2
    exit 1
  fi
  if [[ -e "$root_dir/.venv" || -L "$root_dir/.venv" ]]; then
    backup_dir="$(mktemp -d "$root_dir/.venv-backup.XXXXXX")"
    mv -- "$root_dir/.venv" "$backup_dir/venv"
    printf 'Preserved incompatible environment in %s/venv\n' "$backup_dir"
  fi
  printf 'Creating environment with %s\n' "$selected_python"
  "$selected_python" -m venv "$root_dir/.venv"
fi

"$venv_python" -m pip install --disable-pip-version-check -r "$root_dir/requirements.txt"

yaml_output="$("$venv_python" "$root_dir/yaml/run.py")"
dipl_output="$("$venv_python" "$root_dir/dipl/run.py")"

printf 'YAML version:\n%s\n\nDIPL version:\n%s\n' "$yaml_output" "$dipl_output"

if [[ "$yaml_output" != "$dipl_output" ]]; then
  printf 'The two versions produced different output.\n' >&2
  exit 1
fi
