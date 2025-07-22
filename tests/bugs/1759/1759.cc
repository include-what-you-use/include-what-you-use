//===--- 1759.cc - isyu test ----------------------------------------------===//
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
  return (getFoo() == getFoo()) ? 1 : 0;
}

/**** IWYU_SUMMARY

(tests/bugs/1759/1759.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
