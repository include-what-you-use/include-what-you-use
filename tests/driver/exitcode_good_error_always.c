//===--- exitcode_good_error_always.c - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --error_always

// When --error_always is provided, IWYU exits with error even if analysis
// succeeds.

/**** IWYU_SUMMARY(1)

(tests/driver/exitcode_good_error_always.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
