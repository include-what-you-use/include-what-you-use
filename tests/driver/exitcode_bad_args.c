//===--- exitcode_bad_args.c - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -some -unsupported -Xiwyu -arguments

// When argument parsing fails, IWYU exits with code 1.

/**** IWYU_SUMMARY(1)

***** IWYU_SUMMARY */
