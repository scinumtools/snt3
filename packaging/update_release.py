"""Update archive-based recipes from a real release archive, without publishing."""

import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import tarfile
import urllib.request


ROOT = Path(__file__).resolve().parents[1]


def archive_version(data):
    with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as archive:
        members = [member for member in archive.getmembers()
                   if member.isfile() and len(Path(member.name).parts) == 2
                   and Path(member.name).name == "settings.env"]
        if len(members) != 1:
            raise ValueError("Release archive must contain one top-level settings.env")
        content = archive.extractfile(members[0]).read().decode()
        prefix = Path(members[0].name).parts[0]
        try:
            cmake = archive.extractfile(f"{prefix}/CMakeLists.txt").read().decode()
            main = archive.extractfile(f"{prefix}/apps/snt/main.cpp").read().decode()
        except KeyError as error:
            raise ValueError("Archive is missing unified CLI sources") from error
        if "ENABLE_SNT_SERVER" not in cmake or "ENABLE_SNT_DMAP" not in cmake or "module_server" not in main or "module_dmap" not in main:
            raise ValueError("Archive predates the unified CLI; publish a release containing snt server and snt dmap first")
    match = re.search(r"^CODE_VERSION=([0-9]+\.[0-9]+\.[0-9]+)$", content, re.M)
    if not match:
        raise ValueError("Release archive has no supported CODE_VERSION")
    return match.group(1)


def replace_once(pattern, replacement, text):
    result, count = re.subn(pattern, lambda match: match.expand(replacement), text, flags=re.M)
    if count != 1:
        raise ValueError(f"Expected one recipe field matching {pattern!r}; found {count}")
    return result


def render_updates(root, version, data):
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        raise ValueError("Version must have the form major.minor.patch")
    if archive_version(data) != version:
        raise ValueError("Archive CODE_VERSION does not match the requested release")
    sha256 = hashlib.sha256(data).hexdigest()
    sha512 = hashlib.sha512(data).hexdigest()
    updates = {}
    path = root / "packaging/homebrew/scinumtools3.rb"
    text = replace_once(r'^  url ".*"$',
                        f'  url "https://github.com/scinumtools/snt3/archive/refs/tags/v{version}.tar.gz"',
                        path.read_text())
    updates[path] = replace_once(r'^  sha256 "[a-f0-9]+"$', f'  sha256 "{sha256}"', text)
    path = root / "packaging/vcpkg/portfile.cmake"
    text = replace_once(r'(^    REPO scinumtools/snt3\n)    REF v\S+$',
                        r'\g<1>    REF v' + version, path.read_text())
    updates[path] = replace_once(r'(^    REF v\S+\n)    SHA512 [a-f0-9]+$',
                                 r'\g<1>    SHA512 ' + sha512, text)
    path = root / "packaging/vcpkg/vcpkg.json"
    manifest = json.loads(path.read_text())
    manifest["version"] = version
    manifest.pop("port-version", None)
    updates[path] = json.dumps(manifest, indent=2) + "\n"
    path = root / "packaging/conda-forge/meta.yaml"
    text = replace_once(r'^\{% set version = ".*" %\}$',
                        '{% set version = "' + version + '" %}', path.read_text())
    text = replace_once(r'(^  - url: https://github.com/scinumtools/snt3/archive/refs/tags/v\{\{ version \}\}\.tar\.gz\n)    sha256: [a-f0-9]+$',
                        r'\g<1>    sha256: ' + sha256, text)
    updates[path] = replace_once(r'^  number: [0-9]+$', '  number: 0', text)
    return updates


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("version", help="Published release version, without the v prefix")
    parser.add_argument("--archive", type=Path, help="Use an already downloaded GitHub tag archive")
    parser.add_argument("--check", action="store_true", help="Report stale recipes without editing")
    args = parser.parse_args()
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", args.version):
        parser.error("Version must have the form major.minor.patch")
    if args.archive:
        data = args.archive.read_bytes()
    else:
        url = f"https://github.com/scinumtools/snt3/archive/refs/tags/v{args.version}.tar.gz"
        with urllib.request.urlopen(url, timeout=60) as response:
            data = response.read()
    updates = render_updates(ROOT, args.version, data)
    changed = [path for path, text in updates.items() if path.read_text() != text]
    for path in changed:
        print(path.relative_to(ROOT))
    if args.check:
        return int(bool(changed))
    for path in changed:
        path.write_text(updates[path])
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, tarfile.TarError) as error:
        raise SystemExit(str(error)) from None
