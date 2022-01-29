//===--- exitcode_warn_error_always_arg.c - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --error_always=11

// When --error_always is provided, IWYU exits with error whether or not there
// are IWYU violations. See exitcode_good_error_always_arg.c for complementary
// testcase where there are no violations.

#include "tests/driver/direct.h"

// IWYU: Indirect is...*indirect.h
struct Indirect x;

/**** IWYU_SUMMARY(11)

tests/driver/exitcode_warn_error_always_arg.c should add these lines:
#include "tests/driver/indirect.h"

tests/driver/exitcode_warn_error_always_arg.c should remove these lines:
- #include "tests/driver/direct.h"  // lines XX-XX

The full include-list for tests/driver/exitcode_warn_error_always_arg.c:
#include "tests/driver/indirect.h"  // for Indirect

***** IWYU_SUMMARY */
