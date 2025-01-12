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

template <typename T>
struct TplUsingInDtor {
  ~TplUsingInDtor() {
    T t;
  }
};

class IndirectClass;
using NonProviding = Tpl<IndirectClass>;
using NonProvidingTplUsingInDtor = TplUsingInDtor<IndirectClass>;

struct Struct {
  // IWYU: IndirectClass is...*indirect.h
  ~Struct() = default;

  NonProvidingTplUsingInDtor member;
};

template <typename T>
struct Outer {
  T t;
};

template <typename T>
void FnUsingNestedTpl() {
  Outer<Tpl<T>> o;
}

void Fn() {
  // IWYU: IndirectClass is...*indirect.h
  NonProviding::StaticFn();

  // Test that IWYU doesn't scan Tpl::StaticFn when it is not used.
  // IndirectClass is not fully used here. For this test case, it is important
  // that StaticFn is used in the translation unit so as to be instantiated.
  FnUsingNestedTpl<IndirectClass>();
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
