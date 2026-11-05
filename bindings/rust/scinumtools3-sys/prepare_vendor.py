"""Copy the native source needed by the published -sys crate."""

from pathlib import Path
import shutil


PACKAGE = Path(__file__).resolve().parent
ROOT = PACKAGE.parents[2]
DEST = PACKAGE / "vendor" / "snt3"


def copy_file(relative: str) -> None:
    target = DEST / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(ROOT / relative, target)


if __name__ == "__main__":
    if DEST.exists():
        raise SystemExit(f"{DEST} already exists; start from a clean checkout")
    for directory in ("src", "include", "cmake"):
        shutil.copytree(ROOT / directory, DEST / directory)
    shutil.copytree(ROOT / "bindings/c/include", DEST / "bindings/c/include")
    shutil.copytree(ROOT / "bindings/c/src", DEST / "bindings/c/src")
    for path in (
        "CMakeLists.txt",
        "settings.env",
        "LICENSE",
        "tests/CMakeLists.txt",
        "bindings/c/CMakeLists.txt",
        "bindings/cmake/SNTDIPL.cmake",
        "external/nlohmann/json.hpp",
        "external/nlohmann/LICENSE.MIT",
    ):
        copy_file(path)
    print(f"Prepared SNT3 native source in {DEST}")
