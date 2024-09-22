#!/bin/bash

##===--- iwyu-fixup-llvm-apt.bash -----------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# The apt packages for LLVM at https://apt.llvm.org often break when work starts
# on a major release. This script can recognize and patch up a few common
# issues:
#
# * Unused missing files -- the CMake modules are installed by the llvm-NN-dev
#   and clang-NN packages, but they specify targets for all LLVM components,
#   including ones installed by other packages. Presumably this is due to
#   missing package dependencies or incomplete packages.
# * Broken soname symlinks -- LLVM and Clang soname versions have been in flux
#   lately, and the numbering sometimes fails to match the target names.
#
# Both of these errors manifest as an error from CMake similar to:
#
#     The imported target "xyz" references the file
#
#          "/usr/lib/llvm-NN/lib/libxyz.a"
#
#     but this file does not exist.
#
# This script brutally creates any missing file or link. It's declarative, so we
# only list unused missing files and broken symlinks, and the script does the
# minimal operation possible to fix it up.
#
# It also supports both fixup and revert, so you can clean up your system after
# experimenting, once the upstream packages are repaired.
set -eu
set -o pipefail

LLVM_VER="$1"

if [[ -z "$2" || "$2" == "fixup" ]]; then
    ACTION="fixup"
elif [[ "$2" == "revert" ]]; then
    ACTION="revert"
else
    echo "unknown action: $2" >&2
    exit 1
fi

HOST_TARGET="$(gcc -dumpmachine)"
if [[ -z "$HOST_TARGET" ]]; then
    echo "no GCC host target found" >&2
    exit 1
fi

LLVM_PREFIX="$("llvm-config-$LLVM_VER" --prefix)"
if [[ -z "$LLVM_PREFIX" ]]; then
    echo "no LLVM prefix found for version: $LLVM_VER" >&2
    exit 1
fi

# These are files CMake has targets for but the file is not installed.
# Work around by creating empty placeholder files.
UNUSED_MISSING_FILES=(
    # clang-format-$LLVM_VER
    $LLVM_PREFIX/bin/clang-format
    # clang-tidy-$LLVM_VER
    $LLVM_PREFIX/bin/clang-tidy
    # clang-tools-$LLVM_VER
    $LLVM_PREFIX/bin/amdgpu-arch
    $LLVM_PREFIX/bin/clang-apply-replacements
    $LLVM_PREFIX/bin/clang-change-namespace
    $LLVM_PREFIX/bin/clang-check
    $LLVM_PREFIX/bin/clang-doc
    $LLVM_PREFIX/bin/clang-extdef-mapping
    $LLVM_PREFIX/bin/clang-include-cleaner
    $LLVM_PREFIX/bin/clang-include-fixer
    $LLVM_PREFIX/bin/clang-installapi
    $LLVM_PREFIX/bin/clang-linker-wrapper
    $LLVM_PREFIX/bin/clang-move
    $LLVM_PREFIX/bin/clang-nvlink-wrapper
    $LLVM_PREFIX/bin/clang-offload-bundler
    $LLVM_PREFIX/bin/clang-offload-packager
    $LLVM_PREFIX/bin/clang-query
    $LLVM_PREFIX/bin/clang-refactor
    $LLVM_PREFIX/bin/clang-reorder-fields
    $LLVM_PREFIX/bin/clang-repl
    $LLVM_PREFIX/bin/clang-scan-deps
    $LLVM_PREFIX/bin/clang-tblgen
    $LLVM_PREFIX/bin/diagtool
    $LLVM_PREFIX/bin/find-all-symbols
    $LLVM_PREFIX/bin/modularize
    $LLVM_PREFIX/bin/nvptx-arch
    $LLVM_PREFIX/bin/pp-trace
    $LLVM_PREFIX/bin/sancov
    $LLVM_PREFIX/bin/tblgen-lsp-server
    # clangd-$LLVM_VER
    $LLVM_PREFIX/bin/clangd
    # libllvmlibc-$LLVM_VER-dev
    $LLVM_PREFIX/lib/libLibcTableGenUtil.a
    # libpolly-$LLVM_VER-dev
    $LLVM_PREFIX/lib/libPolly.a
    $LLVM_PREFIX/lib/libPollyISL.a
    # mlir-$LLVM_VER-tools
    $LLVM_PREFIX/bin/mlir-cat
    $LLVM_PREFIX/bin/mlir-cpu-runner
    $LLVM_PREFIX/bin/mlir-linalg-ods-yaml-gen
    $LLVM_PREFIX/bin/mlir-lsp-server
    $LLVM_PREFIX/bin/mlir-minimal-opt
    $LLVM_PREFIX/bin/mlir-minimal-opt-canonicalize
    $LLVM_PREFIX/bin/mlir-opt
    $LLVM_PREFIX/bin/mlir-pdll-lsp-server
    $LLVM_PREFIX/bin/mlir-query
    $LLVM_PREFIX/bin/mlir-reduce
    $LLVM_PREFIX/bin/mlir-transform-opt
    $LLVM_PREFIX/bin/mlir-translate
)

for path in "${UNUSED_MISSING_FILES[@]}"; do
    case "$ACTION" in
        fixup)
            [[ ! -e "$path" ]] && touch "$path" && echo "$path: created"
            ;;
        revert)
            if [[ -e "$path" ]]; then
                if [[ ! -f "$path" ]]; then
                    echo "$path: not a regular file, leaving behind" >&2
                elif [[ -s "$path" ]]; then
                    echo "$path: non-empty, leaving behind" >&2
                else
                    rm "$path" && echo "$path: removed"
                fi
            fi
            ;;
    esac
done

# These are symlinks CMake has targets for but the symlink is not installed
# under the right name. Work around by creating new symlinks as specified using
# "link<space>target" form.
BROKEN_SYMLINKS=(
    # actually installed: $LLVM_PREFIX/lib/libclang-cpp.so.$LLVM_VER.1 -> ../../x86_64-linux-gnu/libclang-cpp.so.$LLVM_VER.1
    "../../$HOST_TARGET/libclang-cpp.so.$LLVM_VER.0 $LLVM_PREFIX/lib/libclang-cpp.so.$LLVM_VER.0"
)

for pair in "${BROKEN_SYMLINKS[@]}"; do
    record=($pair); target=${record[0]}; link=${record[1]}

    case "$ACTION" in
        fixup)
            [[ ! -L "$link" && ! -e "$link" ]] && \
                ln -s "$target" "$link" && \
                echo "$link: linked to $target"
            ;;
        revert)
            if [[ -L "$link" ]]; then
                live_target="$(readlink "$link")"
                if [[ "$target" == "$live_target" ]]; then
                    rm "$link" && echo "$link: removed"
                else
                    echo "$link: target mismatch, expected $target, was $link_target" >&2
                fi
            elif [[ -e "$link" ]]; then
                echo "$link: not a link" >&2
            fi
            ;;
    esac
done
