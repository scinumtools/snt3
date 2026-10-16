"""Calculate retry schedules from parameters evaluated by DIPL; no network calls."""

from pathlib import Path

from scinumtools3.dip import DIP


def attempt_starts(attempts, timeout, delay):
    """Assume every attempt consumes its timeout, with a delay before each retry."""
    return [i * (timeout + delay) for i in range(attempts)]


def main():
    parser = DIP()
    parser.add_file(Path(__file__).with_name("parameters.dip"))
    parameters = parser.parse()

    for name in ("catalog", "search"):
        prefix = f"clients[{name}]"
        attempts = parameters[f"{prefix}.attempts"].value
        timeout = parameters[f"{prefix}.timeout_seconds"].value
        delay = parameters[f"{prefix}.delay_seconds"].value
        duration = parameters[f"{prefix}.max_duration"].value
        budget = parameters[f"{prefix}.budget"].value
        starts = attempt_starts(attempts, timeout, delay)
        print(f"{name} attempt starts = {', '.join(f'{t:.3f}' for t in starts)} s")
        print(f"{name} max duration = {duration:.3f} s (budget {budget:.3f} s)")

    metadata = parameters["clients[search].timeout"].provenance.metadata
    print(f"parameter title = {metadata.title}")
    print(f"description = {metadata.description}")


if __name__ == "__main__":
    main()
