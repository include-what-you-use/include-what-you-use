//===--- main_file_in_internal.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests a main file in /internal/ that includes an /internal/ header of
// another directory. The /internal/ heuristic maps a private header to its
// includer, and clang reports the main file as included by <built-in>, so
// IWYU must not map the main file to <built-in>.

#include "tests/cxx/subdir/internal/main_file_in_internal-d1.h"

Widget w;

/**** IWYU_SUMMARY

(tests/cxx/internal/main_file_in_internal.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
