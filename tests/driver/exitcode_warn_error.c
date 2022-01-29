//===--- exitcode_warn_error.c - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --error

// When --error is provided, IWYU exits with error if analysis finds IWYU
// violations. Default exit code is 1.

#include "tests/driver/direct.h"

// IWYU: Indirect is...*indirect.h
struct Indirect x;

/**** IWYU_SUMMARY(1)

tests/driver/exitcode_warn_error.c should add these lines:
#include "tests/driver/indirect.h"

tests/driver/exitcode_warn_error.c should remove these lines:
- #include "tests/driver/direct.h"  // lines XX-XX

The full include-list for tests/driver/exitcode_warn_error.c:
#include "tests/driver/indirect.h"  // for Indirect

***** IWYU_SUMMARY */
