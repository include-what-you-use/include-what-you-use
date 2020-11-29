//===--- keep_includes.c - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --keep=tests/c/keep_includes*.h -I .

// keep_includes-d1.h is an empty file.
// Normally it would be removed.
// In this test we are exercising the --keep command line option.
// IWYU should not remove keep_include-d1.h when
// --keep=tests/c/keep_includes*.h is used.

#include "keep_includes-d1.h"

/**** IWYU_SUMMARY

(tests/c/keep_includes.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
