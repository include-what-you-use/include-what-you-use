//===--- 1000.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include <cmath>

static void f() {
  (void)std::abs(-12);
}

/**** IWYU_SUMMARY

(tests/bugs/1000/1000.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
