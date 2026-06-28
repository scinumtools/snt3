#!/usr/bin/env bash

set -euo pipefail

source ./settings.env

FILE="scinumtools3-${CODE_VERSION}.tar.gz"
URL="https://github.com/vrtulka23/scinumtools3/archive/refs/tags/${CODE_VERSION}.tar.gz"

curl -L "$URL" -o "$FILE"

echo "SHA256:"
shasum -a 256 "$FILE"

echo

echo "SHA512:"
shasum -a 512 "$FILE"

rm "$FILE"
