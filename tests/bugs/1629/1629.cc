//===--- 1629.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++14
// IWYU_XFAIL

#include "foo.h"

int main() {
  Vector<int> v;
}

/**** IWYU_SUMMARY

(tests/bugs/1629/1629.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
