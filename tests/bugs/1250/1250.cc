//===--- 1250.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -stdlib=libc++
// IWYU_XFAIL

#include "1250.h"

int main() {
  int x[10]{};
  // No diagnostic expected here.
  return get_begin(x);
}

/**** IWYU_SUMMARY

(tests/bugs/1250/1250.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
