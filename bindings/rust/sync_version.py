"""Set Cargo crate versions from settings.env or an explicit release version."""

import argparse
from pathlib import Path
import re
import tomllib


ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "bindings/rust/Cargo.toml"
LOCK = ROOT / "bindings/rust/Cargo.lock"


def source_version() -> str:
    match = re.search(r"^CODE_VERSION=([^\s#]+)$", (ROOT / "settings.env").read_text(), re.M)
    if not match:
        raise SystemExit("CODE_VERSION is missing from settings.env")
    return match.group(1)


def replace_one(text: str, pattern: str, version: str) -> str:
    updated, count = re.subn(
        pattern, lambda match: match.group(1) + version + match.group(2), text
    )
    if count != 1:
        raise SystemExit(f"Expected exactly one match for {pattern}")
    return updated


def sync(version: str) -> None:
    manifest = MANIFEST.read_text()
    manifest = replace_one(manifest, r'(?m)^(version = ")[^"]+("\n)', version)
    for crate in ("scinumtools3-sys", "scinumtools3", "scinumtools3-build"):
        manifest = replace_one(
            manifest,
            rf'(?m)^({crate} = \{{ version = "=)[^"]+("[^\n]*\n)',
            version,
        )
    MANIFEST.write_text(manifest)

    lock = LOCK.read_text()
    for crate in ("scinumtools3", "scinumtools3-sys", "scinumtools3-build"):
        lock = replace_one(
            lock, rf'(?m)^(name = "{crate}"\nversion = ")[^"]+("\n)', version
        )
    LOCK.write_text(lock)


def check(version: str) -> None:
    manifest = tomllib.loads(MANIFEST.read_text())
    lock = tomllib.loads(LOCK.read_text())
    actual = manifest["workspace"]["package"]["version"]
    dependencies = manifest["workspace"]["dependencies"]
    crates = ("scinumtools3-sys", "scinumtools3", "scinumtools3-build")
    locked = {
        package["name"]: package["version"]
        for package in lock["package"]
        if package["name"] in crates
    }
    if (
        actual != version
        or any(dependencies[crate]["version"] != f"={version}" for crate in crates)
        or locked != {crate: version for crate in crates}
    ):
        raise SystemExit(
            f"Rust versions do not match SNT3 {version}; "
            "run python3 bindings/rust/sync_version.py --write"
        )
    print(f"Rust crates are version {version}; source CODE_VERSION is {source_version()}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="update Cargo versions")
    parser.add_argument("--version", help="crate version to use instead of CODE_VERSION")
    args = parser.parse_args()
    version = args.version or source_version()
    if not re.fullmatch(
        r"(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)"
        r"(?:-[0-9A-Za-z.-]+)?(?:\+[0-9A-Za-z.-]+)?",
        version,
    ):
        raise SystemExit(f"Invalid Cargo version: {version}")
    if args.write:
        sync(version)
    check(version)
