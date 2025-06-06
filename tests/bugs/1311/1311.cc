//===--- 1311.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "s.h"

// #include <unordered_set>

void f(const S& s) {
  // No diagnostic expected.
  for (int i : s.i) {
  }
}

/**** IWYU_SUMMARY

(tests/bugs/1311/1311.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
