//===--- exitcode_good_error.c - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --error

// When --error is provided, IWYU exits with error only if analysis finds IWYU
// violations. In this case there are none, so IWYU exits with 0.

/**** IWYU_SUMMARY(0)

(tests/driver/exitcode_good_error.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
