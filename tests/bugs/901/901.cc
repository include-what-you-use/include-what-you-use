//===--- 901.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL
#include "b.h"

int f() {
  MyData data{};
  return data.meters.value;
}

/**** IWYU_SUMMARY

(tests/bugs/901/901.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
