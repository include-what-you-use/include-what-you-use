//===--- stdint.cc - test input file for IWYU -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU does some heuristic name matching to find the "associated header"; the
// header declaring the public API for the current source file, e.g. "foo.h" for
// "foo.cc". Make sure it doesn't consider system headers associated.
//
// If it fails, this test will print unwanted IWYU diagnostics from stdint.h.

#include <stdint.h>

int foo() {
  return (int32_t)100;
}

/**** IWYU_SUMMARY

(tests/cxx/stdint.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
