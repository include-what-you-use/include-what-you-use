//===--- 228.cc - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I tests/bugs/228
// IWYU_XFAIL

#include <228.hpp>

void B::doSomethingMore() {
  A::doSomething();
}

/**** IWYU_SUMMARY

(tests/bugs/228/228.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
