#!/bin/bash

# This script checks all versions in the repository.
# Run this from the source root directory:
# ./devtools/version_check.sh

DIR_SOURCE=.

echo "pyproject.toml                   version from settings.env (dynamic metadata)"
echo "settings.env                    " $(grep CODE_VERSION $DIR_SOURCE/settings.env)
echo "docs/dipl/specification.md      " $(grep '^version:' $DIR_SOURCE/docs/dipl/specification.md)
echo "docs/puel/specification.md      " $(grep '^version:' $DIR_SOURCE/docs/puel/specification.md)
