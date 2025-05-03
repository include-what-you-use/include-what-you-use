//===--- 801.cc - iwyu test -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_XFAIL

struct Foo {
  static constexpr int num = 10;
};

#include "num_getter.h"

#include "int_wrapper_foo.h"

int dummy() {
  NumGetter<Foo> x;
  return x.GetNum();
}

/**** IWYU_SUMMARY

(tests/bugs/801/801.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
