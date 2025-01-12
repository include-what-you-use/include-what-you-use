#!/bin/bash

##===--- iwyu-fixup-llvm-target-checks.bash -------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# CMake has sophisticated support for exporting targets pointing to installed
# files. It also verifies at cmake configure time that all exported targets
# actually exist on disk. There is no documented way to disable that check.
#
# The way apt.llvm.org packages LLVM and Clang, CMake can't quite keep up. It
# knows about all targets and where they were installed by the 'install' step,
# not how later packaging combines the files into more fine-grained .deb
# packages.
#
# So the CMake modules installed will know about the sum total of all targets
# built, and will attempt to verify that the files they produced exist. But the
# packages containing the CMake modules (llvm-NN-dev and clang-NN) do not
# contain all these files, so the verification would nominally always fail.
#
# There is some apriori fixup in the packaging scripts to work around this [1],
# but it often falls behind changes to upstream such as new components being
# added that are not part of the primary packages.
#
# This script comments out enough CMake-generated metadata to disable all the
# builtin checks. For our purposes, it's better if the build fails later due to
# actual missing dependencies than the cmake step failing due to _unused_
# missing dependencies.
#
# [1]: https://salsa.debian.org/pkg-llvm-team/llvm-toolchain/-/blob/2537c98/debian/rules#L1368

set -eu
set -o pipefail

if [[ $# != 1 ]]; then
    echo "usage: $0 LLVM_INSTALL_PREFIX"
    echo ""
    echo "  LLVM_INSTALL_PREFIX    install root of llvm, e.g. /usr/lib/llvm-20"
    exit 1
fi

LLVM_PREFIX="$1"
TIMESTAMP=$(date +%Y%m%d%H%M%S)

# The variable name changed from _IMPORT_CHECK_FILES_FOR_xxx to
# _cmake_import_check_files_for_xxx in CMake 3.24:
# https://gitlab.kitware.com/cmake/cmake/-/commit/59cc92085ea7c5a81de7d79946fa97f045adea78
# so check for both.

sed \
    -i".$TIMESTAMP" \
    -e '/^list(APPEND _\(IMPORT_CHECK_FILES_FOR\|cmake_import_check_files_for\)_/ {s|^|#|}' \
    $LLVM_PREFIX/lib/cmake/llvm/LLVMExports-*.cmake \
    $LLVM_PREFIX/lib/cmake/clang/ClangTargets-*.cmake
