#!/bin/bash

##===--- iwyu-roll-googletest.bash ----------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# This script imports a specified version (or latest main) of googletest into
# the IWYU source tree as a vendored library. We build and link it in the main
# CMakeLists.txt

set -e

cd "$(dirname "$0")"

# Clone into a temporary working tree, optionally check out tag.
git clone git@github.com:google/googletest.git ./_googletest

# Check out requested ref or stay on default.
if [[ -n "$1" ]]; then
    TAG="$1"
else
    TAG=$(git -C ./_googletest rev-parse HEAD)
fi
git -C ./_googletest -c advice.detachedHead=false checkout "$TAG"

# Recreate vendor directory.
rm -rf ./googletest
mkdir ./googletest

# Flatten the googletest project tree into the top-level.
cp -p ./_googletest/LICENSE ./googletest/
cp -Rp ./_googletest/googletest/src ./googletest/
cp -Rp ./_googletest/googletest/include ./googletest/

# Leave a version marker.
echo "$TAG" > ./googletest/VERSION.iwyu

# Remove temporary working tree.
rm -rf ./_googletest
