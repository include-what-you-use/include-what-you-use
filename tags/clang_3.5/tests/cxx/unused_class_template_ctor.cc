//===--- unused_class_template_ctor.cc - test input file for iwyu ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that IWYU doesn't instantiate unused class template constructor
// because for certain template arguments some constructors can trigger Clang
// errors.

#include "tests/cxx/unused_class_template_ctor-d1.h"

void foo() {
  int i = 10, j = 20;
  pair<int&, int&> p(i, j);
}

/**** IWYU_SUMMARY

(tests/cxx/unused_class_template_ctor.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
