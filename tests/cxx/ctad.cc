//===--- ctad.cc - test input file for iwyu -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++17

// Test that C++17 CTAD (Class Template Argument Deduction) is recognized as
// pointing back to a template, even if it's not explicitly specialized.

#include "tests/cxx/ctad-d1.h"

void f() {
  // IWYU: Deduced is...*ctad-i1.h
  Deduced d([] {});
}

/**** IWYU_SUMMARY

tests/cxx/ctad.cc should add these lines:
#include "tests/cxx/ctad-i1.h"

tests/cxx/ctad.cc should remove these lines:
- #include "tests/cxx/ctad-d1.h"  // lines XX-XX

The full include-list for tests/cxx/ctad.cc:
#include "tests/cxx/ctad-i1.h"  // for Deduced

***** IWYU_SUMMARY */
