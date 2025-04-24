//===--- 1122.cc - test input file for iwyu -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Adapted from:
// https://github.com/graveljp/iwyu-bugs/tree/main/operator_overload_macro

// IWYU_XFAIL

#include "utils.h"

int main() {
  return TimesTen(3);
}

/**** IWYU_SUMMARY

(tests/bugs/1122/1122.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
