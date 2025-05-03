//===--- 1127b.cc - iwyu test ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include <iostream>
#include "lib.h"

int main() {
  const auto my_set = make_set();
  for (auto& e : my_set) {
    std::cout << e << '\n';
  }
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/1127b/1127b.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
