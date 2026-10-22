#!/bin/sh
# Install the precompiled snt CLI without administrator privileges.
set -eu

usage() {
    cat <<'EOF'
Usage: sh install-cli.sh [--version vX.Y.Z] [--prefix DIR] [--archive FILE]

Installs the latest release into ~/.local by default. --archive installs an
already downloaded release archive, for example on an offline HPC system.
EOF
}

fail() {
    printf 'snt install: %s\n' "$1" >&2
    exit 1
}

tag=latest
archive_path=
prefix=${SNT_INSTALL_PREFIX:-"$HOME/.local"}
while [ "$#" -gt 0 ]; do
    case "$1" in
        --version|--prefix|--archive)
            option=$1
            shift
            [ "$#" -gt 0 ] || fail "$option requires a value"
            case "$option" in
                --version) tag=$1 ;;
                --prefix) prefix=$1 ;;
                --archive) archive_path=$1 ;;
            esac
            ;;
        --help|-h) usage; exit 0 ;;
        *) usage >&2; fail "unknown option: $1" ;;
    esac
    shift
done

case "$(uname -s):$(uname -m)" in
    Linux:x86_64) platform=linux-x86_64 ;;
    Darwin:arm64) platform=macos-arm64 ;;
    *) fail "no precompiled CLI archive for $(uname -s) $(uname -m)" ;;
esac

if [ -n "$archive_path" ]; then
    [ -f "$archive_path" ] || fail "archive not found: $archive_path"
    archive_name=${archive_path##*/}
    case "$archive_name" in
        snt-v*-"$platform".tar.gz)
            archive_tag=${archive_name#snt-}
            archive_tag=${archive_tag%-"$platform".tar.gz}
            ;;
        *) fail "expected an snt-<version>-$platform.tar.gz archive" ;;
    esac
    if [ "$tag" != latest ] && [ "$tag" != "$archive_tag" ]; then
        fail "archive version $archive_tag does not match $tag"
    fi
    tag=$archive_tag
else
    command -v curl >/dev/null 2>&1 || fail "curl is required to download a release"
    if [ "$tag" = latest ]; then
        release_url=$(curl -fsSLI -o /dev/null -w '%{url_effective}' \
            https://github.com/scinumtools/snt3/releases/latest) || fail "could not find the latest release"
        tag=${release_url##*/}
    fi
    archive_name="snt-${tag}-${platform}.tar.gz"
fi

case "$tag" in
    v[0-9]*.[0-9]*.[0-9]*) ;;
    *) fail "invalid release tag: $tag" ;;
esac
case "$tag" in
    *[!A-Za-z0-9._-]*) fail "invalid release tag: $tag" ;;
esac

mkdir -p "$prefix/opt/snt" "$prefix/bin"
prefix=$(cd "$prefix" && pwd -P)
target="$prefix/opt/snt/$tag"
link="$prefix/bin/snt"
if [ -e "$link" ] && [ ! -L "$link" ]; then
    fail "will not replace an existing file: $link"
fi

if [ ! -d "$target" ]; then
    stage=$(mktemp -d "$prefix/opt/snt/.install.XXXXXXXX") || fail "could not create staging directory"
    trap 'rm -rf "$stage"' EXIT HUP INT TERM
    if [ -z "$archive_path" ]; then
        archive_path="$stage/$archive_name"
        curl -fL --retry 3 \
            "https://github.com/scinumtools/snt3/releases/download/$tag/$archive_name" \
            -o "$archive_path" || fail "could not download $archive_name"
    fi
    tar -xzf "$archive_path" -C "$stage" || fail "could not extract $archive_name"
    extracted="$stage/snt-${tag}-${platform}"
    [ -x "$extracted/snt" ] || fail "archive does not contain an executable snt"
    "$extracted/snt" --version >/dev/null || fail "snt could not run; check system runtime libraries"
    mv "$extracted" "$target"
fi

[ -x "$target/snt" ] || fail "installation is incomplete: $target"
ln -sfn "$target/snt" "$link"
printf 'Installed %s\n' "$link"
case ":$PATH:" in
    *":$prefix/bin:"*) ;;
    *) printf 'Add %s/bin to PATH to run snt by name.\n' "$prefix" ;;
esac
