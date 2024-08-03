//===--- template_member_functions.cc - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests scanning class template member functions.

#include "tests/cxx/direct.h"

template <typename T>
struct Tpl {
  static void StaticFn() {
    T t;
  }
};

class IndirectClass;
using NonProviding = Tpl<IndirectClass>;

void Fn() {
  // IWYU: IndirectClass is...*indirect.h
  NonProviding::StaticFn();
}

/**** IWYU_SUMMARY

tests/cxx/template_member_functions.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/template_member_functions.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/template_member_functions.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
