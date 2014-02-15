//===--- overloaded_class.cc - test input file for iwyu -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the case where a template type is instantiated inside a
// template function.  If all the possible instantiations come
// from the same place, we want the template function to be
// responsible for the template type, and not the caller.

#include "tests/cxx/overloaded_class-d1.h"

int main() {
  // IWYU: MyFunc is...*overloaded_class-i1.h
  MyFunc<int>();
}

/**** IWYU_SUMMARY

tests/cxx/overloaded_class.cc should add these lines:
#include "tests/cxx/overloaded_class-i1.h"

tests/cxx/overloaded_class.cc should remove these lines:
- #include "tests/cxx/overloaded_class-d1.h"  // lines XX-XX

The full include-list for tests/cxx/overloaded_class.cc:
#include "tests/cxx/overloaded_class-i1.h"  // for MyFunc

***** IWYU_SUMMARY */
