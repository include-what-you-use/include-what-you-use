//===--- check_also_sibling.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also=tests/cxx/check_also_sibling.inc -I .

// Tests that a file reported on through '--check_also' has its own
// IWYU_SUMMARY checked, even when its extension differs from this file's.

#include "tests/cxx/check_also_sibling.inc"

void Use() {
  SiblingFn();
}

/**** IWYU_SUMMARY

(tests/cxx/check_also_sibling.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
