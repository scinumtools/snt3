"""Numerical heat solver shared by the YAML and DIPL runners."""

MATERIAL_NAMES = ("baseline", "trial")


def heat_step(temperatures, factor):
    """Advance the interior of a rod with fixed-temperature endpoints."""
    updated = temperatures[:]
    for i in range(1, len(temperatures) - 1):
        updated[i] = temperatures[i] + factor * (
            temperatures[i - 1] - 2 * temperatures[i] + temperatures[i + 1]
        )
    return updated


def simulate(*, run_id, cells, steps, initial, boundary, timestep, factors, title, description):
    """Run both material cases and format their common output."""
    lines = [f"run id = {run_id}"]
    for name in MATERIAL_NAMES:
        temperatures = [boundary] + [initial] * (cells - 1) + [boundary]
        for _ in range(steps):
            temperatures = heat_step(temperatures, factors[name])
        lines.append(f"{name} center_temperature = {temperatures[cells // 2]:.3f} K")
    lines.extend(
        (f"timestep = {timestep:.3f} s", f"parameter title = {title}", f"description = {description}")
    )
    return "\n".join(lines)
