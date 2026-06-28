#!/bin/bash

# This script checks all versions in the repository.
# Run this from the source root directory:
# ./devtools/check_versions.sh

DIR_SOURCE=.

echo "pyproject.toml                  " $(grep "^version =" $DIR_SOURCE/pyproject.toml)
echo "settings.env                    " $(grep CODE_VERSION $DIR_SOURCE/settings.env)
echo "packaging/conda/meta.yaml       " $(grep "version =" $DIR_SOURCE/packaging/conda/meta.yaml)
echo "packaging/vcpkg/portfile.cmake  " $(grep "REF " $DIR_SOURCE/packaging/vcpkg/portfile.cmake)
echo "docs/dipl/specification.md      " $(grep "Version:" $DIR_SOURCE/docs/dipl/specification.md) 
echo "docs/puel/specification.md      " $(grep "Version:" $DIR_SOURCE/docs/puel/specification.md) 
