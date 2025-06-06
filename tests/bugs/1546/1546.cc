//===--- 1546.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "macro.h"

int main() {
  MY_MACRO();
  return 0;
}

/**** IWYU_SUMMARY

(tests/bugs/1546/1546.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
