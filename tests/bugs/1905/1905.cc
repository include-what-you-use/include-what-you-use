//===--- 1905.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
// IWYU_ARGS: -Xiwyu --check_also=tests/bugs/1905/fn.h

#include "fn.h"

void User() {
  Fn1(5);
  Fn2(7);
}

/**** IWYU_SUMMARY

(tests/bugs/1905/1905.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
