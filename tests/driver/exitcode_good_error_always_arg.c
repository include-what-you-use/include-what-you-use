//===--- exitcode_good_error_always_arg.c - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --error_always=100

// When --error_always is provided, IWYU exits with error even if analysis
// succeeds.

/**** IWYU_SUMMARY(100)

(tests/driver/exitcode_good_error_always_arg.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
