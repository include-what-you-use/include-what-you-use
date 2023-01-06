//===--- using_overload_multiple.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we use multiple function overloads through a using decl,
// that we correctly include all of the necessary files for the overload and
// don't accidentally remove files greedily.

#include "using_overload-float.h"
#include "using_overload-int.h"

void use_overload() {
  int a = 1;
  int b = 2;
  float c = 1.f;
  float d = 2.f;
  using ns::add;
  add(a, b);
  add(c, d);
}

/**** IWYU_SUMMARY

(tests/cxx/using_overload_multiple.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
