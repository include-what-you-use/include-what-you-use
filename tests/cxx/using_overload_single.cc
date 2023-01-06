//===--- using_overload_single.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we use a function overload through a using decl, that we
// are only required to include the files that represent the overloads we've
// actually used and not the entire overload set the the using decl
// represents.

#include "using_overload-float.h"
#include "using_overload-int.h"

void use_overload() {
  int a = 1;
  int b = 2;
  using ns::add;
  add(a, b);
}

/**** IWYU_SUMMARY

tests/cxx/using_overload_single.cc should add these lines:

tests/cxx/using_overload_single.cc should remove these lines:
- #include "using_overload-float.h"  // lines XX-XX

The full include-list for tests/cxx/using_overload_single.cc:
#include "using_overload-int.h"  // for add

***** IWYU_SUMMARY */
