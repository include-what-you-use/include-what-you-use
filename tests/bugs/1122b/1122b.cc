//===--- 1122b.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Adapted from:
// https://github.com/graveljp/iwyu-bugs/tree/main/operator_overload

// IWYU_XFAIL

#include "foo.h"

int main() {
  return Foo<int>();
}

/**** IWYU_SUMMARY

(tests/bugs/1122b/1122b.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
