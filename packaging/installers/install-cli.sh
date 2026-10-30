#!/bin/sh
# Install or uninstall the precompiled snt CLI without administrator privileges.
set -eu

usage() {
    cat <<'EOF'
Usage: sh install-cli.sh [--version vX.Y.Z] [--prefix DIR] [--archive FILE]
       sh install-cli.sh --uninstall [--version vX.Y.Z] [--prefix DIR]

Installs the latest release into ~/.local by default. --archive accepts a
multi-platform .tar.gz or .zip bundle, or a legacy platform archive, for
example on an offline HPC system.
--uninstall removes all installed CLI releases by default, or one release when
--version is given. It does not require a network connection.
EOF
}

fail() {
    printf 'snt install: %s\n' "$1" >&2
    exit 1
}

check_linux_runtime() {
    [ "$platform" = linux-x86_64 ] || return 0
    command -v ldd >/dev/null 2>&1 || return 0
    dependency_report=$(ldd "$1" 2>&1) || :
    missing=$(printf '%s\n' "$dependency_report" |
        sed -n 's/^[[:space:]]*\([^[:space:]]*\)[[:space:]]*=>[[:space:]]*not found.*$/\1/p')
    if [ -n "$missing" ]; then
        printf 'snt install: missing system libraries required by snt:\n%s\n' "$missing" >&2
        fail 'install the corresponding runtime packages, then run this installer again'
    fi
    if printf '%s\n' "$dependency_report" | grep -Fq 'no version information available'; then
        printf 'snt install: warning: a system library lacks the symbol version expected by this release; check libcurl compatibility if snt fails to run\n' >&2
    fi
}

valid_tag() {
    printf '%s\n' "$1" | LC_ALL=C grep -Eq '^v[0-9]+\.[0-9]+\.[0-9]+$'
}

tag=latest
archive_path=
uninstall=0
version_given=0
prefix=${SNT_INSTALL_PREFIX:-"$HOME/.local"}
while [ "$#" -gt 0 ]; do
    case "$1" in
        --uninstall) uninstall=1 ;;
        --version|--prefix|--archive)
            option=$1
            shift
            [ "$#" -gt 0 ] || fail "$option requires a value"
            case "$option" in
                --version) tag=$1; version_given=1 ;;
                --prefix) prefix=$1 ;;
                --archive) archive_path=$1 ;;
            esac
            ;;
        --help|-h) usage; exit 0 ;;
        *) usage >&2; fail "unknown option: $1" ;;
    esac
    shift
done

if [ "$uninstall" -eq 1 ]; then
    [ -z "$archive_path" ] || fail "--archive cannot be used with --uninstall"
    if [ "$version_given" -eq 1 ]; then
        valid_tag "$tag" || fail "invalid release tag: $tag"
    fi
    if [ ! -d "$prefix" ]; then
        printf 'No snt CLI installation found under %s\n' "$prefix"
        exit 0
    fi
    prefix=$(cd "$prefix" && pwd -P)
    root="$prefix/opt/snt"
    link="$prefix/bin/snt"
    active_tag=
    if [ -L "$link" ]; then
        link_target=$(readlink "$link") || fail "could not inspect $link"
        case "$link_target" in
            "$root"/*/snt)
                active_tag=${link_target#"$root"/}
                active_tag=${active_tag%/snt}
                valid_tag "$active_tag" || active_tag=
                ;;
        esac
    fi
    removed=0
    if [ -n "$active_tag" ] && { [ "$version_given" -eq 0 ] || [ "$active_tag" = "$tag" ]; }; then
        rm "$link"
        removed=1
    fi
    if [ "$version_given" -eq 1 ]; then
        if [ -d "$root/$tag" ]; then
            rm -rf "$root/$tag"
            removed=1
        fi
    else
        for target in "$root"/v*; do
            [ -d "$target" ] || continue
            candidate_tag=${target##*/}
            valid_tag "$candidate_tag" || continue
            rm -rf "$target"
            removed=1
        done
    fi
    rmdir "$root" "$prefix/opt" "$prefix/bin" 2>/dev/null || :
    if [ "$removed" -eq 1 ]; then
        printf 'Uninstalled snt CLI from %s\n' "$prefix"
    else
        printf 'No matching snt CLI release found under %s\n' "$prefix"
    fi
    exit 0
fi

case "$(uname -s):$(uname -m)" in
    Linux:x86_64) platform=linux-x86_64 ;;
    Darwin:arm64) platform=macos-arm64 ;;
    *) fail "no precompiled CLI archive for $(uname -s) $(uname -m)" ;;
esac

if [ -n "$archive_path" ]; then
    [ -f "$archive_path" ] || fail "archive not found: $archive_path"
    archive_name=${archive_path##*/}
    case "$archive_name" in
        snt-v*-bundle.tar.gz|snt-v*-bundle.zip)
            archive_tag=${archive_name#snt-}
            archive_tag=${archive_tag%-bundle.*}
            ;;
        snt-v*-"$platform".tar.gz)
            archive_tag=${archive_name#snt-}
            archive_tag=${archive_tag%-"$platform".tar.gz}
            ;;
        *) fail "expected an snt-<version>-bundle.tar.gz, .zip, or $platform.tar.gz archive" ;;
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
    archive_name="snt-${tag}-bundle.tar.gz"
fi

valid_tag "$tag" || fail "invalid release tag: $tag"

mkdir -p "$prefix/opt/snt" "$prefix/bin"
prefix=$(cd "$prefix" && pwd -P)
target="$prefix/opt/snt/$tag"
link="$prefix/bin/snt"
if [ -L "$link" ]; then
    link_target=$(readlink "$link") || fail "could not inspect $link"
    case "$link_target" in
        "$prefix"/opt/snt/*/snt) ;;
        *) fail "will not replace an existing symlink: $link" ;;
    esac
elif [ -e "$link" ]; then
    fail "will not replace an existing file: $link"
fi

if [ ! -d "$target" ]; then
    stage=$(mktemp -d "$prefix/opt/snt/.install.XXXXXXXX") || fail "could not create staging directory"
    trap 'rm -rf "$stage"' EXIT HUP INT TERM
    if [ -z "$archive_path" ]; then
        archive_path="$stage/$archive_name"
        if ! curl -fL --retry 3 \
            "https://github.com/scinumtools/snt3/releases/download/$tag/$archive_name" \
            -o "$archive_path"; then
            archive_name="snt-${tag}-bundle.zip"
            archive_path="$stage/$archive_name"
            if ! curl -fL --retry 3 \
                "https://github.com/scinumtools/snt3/releases/download/$tag/$archive_name" \
                -o "$archive_path"; then
                archive_name="snt-${tag}-${platform}.tar.gz"
                archive_path="$stage/$archive_name"
                curl -fL --retry 3 \
                    "https://github.com/scinumtools/snt3/releases/download/$tag/$archive_name" \
                    -o "$archive_path" || fail "could not download $archive_name"
            fi
        fi
    fi
    case "$archive_name" in
        *-bundle.tar.gz)
            platform_archive="$stage/snt-${tag}-${platform}.tar.gz"
            tar -xOzf "$archive_path" "snt-${tag}-${platform}.tar.gz" > "$platform_archive" ||
                fail "bundle does not contain the $platform archive"
            tar -xzf "$platform_archive" -C "$stage" || fail "could not extract the $platform archive"
            ;;
        *-bundle.zip)
            command -v unzip >/dev/null 2>&1 || fail "unzip is required to install the release bundle"
            platform_archive="$stage/snt-${tag}-${platform}.tar.gz"
            unzip -p "$archive_path" "snt-${tag}-${platform}.tar.gz" > "$platform_archive" ||
                fail "bundle does not contain the $platform archive"
            tar -xzf "$platform_archive" -C "$stage" || fail "could not extract the $platform archive"
            ;;
        *) tar -xzf "$archive_path" -C "$stage" || fail "could not extract $archive_name" ;;
    esac
    extracted="$stage/snt-${tag}-${platform}"
    [ -x "$extracted/snt" ] || fail "archive does not contain an executable snt"
    check_linux_runtime "$extracted/snt"
    "$extracted/snt" --version >/dev/null || fail "snt could not run; check system runtime libraries"
    mv "$extracted" "$target"
else
    check_linux_runtime "$target/snt"
fi

[ -x "$target/snt" ] || fail "installation is incomplete: $target"
ln -sfn "$target/snt" "$link"
printf 'Installed %s\n' "$link"
case ":$PATH:" in
    *":$prefix/bin:"*) ;;
    *) printf 'Add %s/bin to PATH to run snt by name.\n' "$prefix" ;;
esac
