"""Evaluate the YAML and DIPL inputs serially with one numerical solver."""

from run_yaml import evaluate as evaluate_yaml
from run_dipl import evaluate as evaluate_dipl


def main():
    yaml_output = evaluate_yaml()
    print(f"YAML version:\n{yaml_output}\n")

    dipl_output = evaluate_dipl()
    print(f"DIPL version:\n{dipl_output}")

    if yaml_output != dipl_output:
        raise SystemExit("The two versions produced different output.")


if __name__ == "__main__":
    main()
