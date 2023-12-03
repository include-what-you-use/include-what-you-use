//===--- multiple_inputs.c - test input file for --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that include-what-you-use exits with error and diagnostic for
// multiple inputs. Pass the current filename three times (first is implicit,
// following two provided by args-line below).

// IWYU_ARGS: tests/driver/multiple_inputs.c tests/driver/multiple_inputs.c

// IWYU~: expected exactly one compiler job

/**** IWYU_SUMMARY(1)

// No IWYU summary expected.

***** IWYU_SUMMARY */
