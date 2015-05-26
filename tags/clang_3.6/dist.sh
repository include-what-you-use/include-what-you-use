#!/bin/sh
##===--------- dist.sh - package up binary distribution of IWYU -----------===##
#
#                      The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

set -eu

# Create clean dist directory
rm -rf dist
mkdir dist

# Assume that Release+Asserts build exists.
ln -f ../../../../../build/Release+Asserts/bin/include-what-you-use dist/
ln -f fix_includes.py dist/
ln -f README.txt dist/
ln -f LICENSE.TXT dist/

pushd dist > /dev/null

# Build archive name to the following pattern:
# include-what-you-use-<ver>-<arch>-<distro>-<release>.tar.gz
ARCHIVE=include-what-you-use-3.3-$(uname -m)-$(lsb_release -si)-$(lsb_release -sr).tar.gz
tar -cvzf $ARCHIVE *

popd > /dev/null

