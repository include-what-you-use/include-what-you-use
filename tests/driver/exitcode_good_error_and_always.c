//===--- exitcode_good_error_and_always.c - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --error=19 -Xiwyu --error_always=91

// When both are provided, --error_always takes precedence

/**** IWYU_SUMMARY(91)

(tests/driver/exitcode_good_error_and_always.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
