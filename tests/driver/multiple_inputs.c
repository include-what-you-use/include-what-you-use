//===--- multiple_inputs.c - test input file for --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that include-what-you-use allows multiple inputs but discards all but
// one, with a diagnostic. The multiple_inputs.c file is already implicitly
// added to IWYU command, use IWYU_ARGS to add it twice more.

// IWYU_ARGS: tests/driver/multiple_inputs.c tests/driver/multiple_inputs.c

// IWYU~: ignoring 2 extra jobs

/**** IWYU_SUMMARY

(tests/driver/multiple_inputs.c has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
