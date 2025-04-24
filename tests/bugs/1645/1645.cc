//===--- 1645.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "bar.h"

int main() {
  bar();
  foo_t foo{};
}

/**** IWYU_SUMMARY

(tests/bugs/1645/1645.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
