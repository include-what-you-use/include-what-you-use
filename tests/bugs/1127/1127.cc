//===--- 1127.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "c.h"

int main() {
  auto i = getIntArray();
  auto i2 = i;
  auto s = i.size();
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/1127/1127.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
