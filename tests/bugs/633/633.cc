//===--- 633.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Shows that IWYU crashes with --check_also="*"; there's something about
// matching all possible files that's tripping up our include-name handling.

// IWYU_ARGS: -Xiwyu --check_also="*"
// IWYU_XFAIL

void nothing();

/**** IWYU_SUMMARY

(tests/bugs/633/633.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
