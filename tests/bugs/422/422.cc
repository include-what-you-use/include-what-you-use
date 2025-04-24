//===--- 422.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

#include "bar.h"
template <typename T>
void do_something() {
  Foo<T> f;
}

void baz() {
  do_something<int>();
}

/**** IWYU_SUMMARY

(tests/bugs/422/422.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
