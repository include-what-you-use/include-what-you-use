//===--- 1349.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "vector.h"

int main() {
  Vector<int> v{1, 2, 3};

  (void)v.begin();
}

/**** IWYU_SUMMARY

(tests/bugs/1349/1349.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
