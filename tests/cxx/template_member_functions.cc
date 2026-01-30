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
#include "tests/cxx/template_member_functions-direct.h"

template <typename T>
struct Tpl {
  static void StaticFn() {
    T t;
  }

  void Fn() {
    T t;
  }

  void operator*() {
    T t;
  }
};

template <typename T1, typename T2>
struct DerivedFromTpl : Tpl<T1> {
  void UseT2() {
    T2 t;
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

TplUsingInDtor<IndirectClass> GetTplUsingInDtor();
ProvidingTplUsingInDtor GetProvidingTplUsingInDtor();
NonProvidingTplUsingInDtor GetNonProvidingTplUsingInDtor();

template <typename T>
TplUsingInDtor<T> TplGetTplUsingInDtor1() {
  return TplUsingInDtor<T>();
}

template <typename T>
TplUsingInDtor<T> TplGetTplUsingInDtor2() {
  return TplUsingInDtor<T>{};
}

template <typename T>
TplUsingInDtor<T> TplGetTplUsingInDtor3() {
  return {};
}

template <typename T = IndirectClass>
TplUsingInDtor<T> TplGetTplUsingInDtorDefArgNonProviding() {
  return TplUsingInDtor<T>();
}

void Fn() {
  // IWYU: IndirectClass is...*indirect.h
  NonProviding::StaticFn();

  // Test that IWYU doesn't scan Tpl::StaticFn when it is not used.
  // IndirectClass is not fully used here. For this test case, it is important
  // that StaticFn is used in the translation unit so as to be instantiated.
  FnUsingNestedTpl<IndirectClass>();

  // IWYU: IndirectClass is...*indirect.h
  (void)TplUsingInDtor<IndirectClass>();
  // TODO: IWYU: IndirectClass is...*indirect.h
  (void)TplUsingInDtor<IndirectClass>{};

  // IWYU: IndirectClass is...*indirect.h
  GetTplUsingInDtor();
  GetProvidingTplUsingInDtor();
  // IWYU: IndirectClass is...*indirect.h
  GetNonProvidingTplUsingInDtor();

  // IWYU: IndirectClass is...*indirect.h
  TplGetTplUsingInDtor1<IndirectClass>();
  // TODO: IWYU: IndirectClass is...*indirect.h
  TplGetTplUsingInDtor2<IndirectClass>();
  // TODO: IWYU: IndirectClass is...*indirect.h
  TplGetTplUsingInDtor3<IndirectClass>();

  // IWYU: IndirectClass is...*indirect.h
  TplGetTplUsingInDtorDefArgNonProviding();
  TplGetTplUsingInDtorDefArgProviding();

  // IWYU: IndirectTemplate needs a declaration
  DerivedFromTpl<IndirectClass, IndirectTemplate<int>> dft;
  // IWYU: IndirectClass is...*indirect.h
  dft.Fn();
  // IWYU: IndirectClass is...*indirect.h
  (dft).Fn();
  extern Tpl<IndirectClass>& tpl;
  // Test that IWYU doesn't ignore explicit type casts.
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  static_cast<DerivedFromTpl<IndirectClass, IndirectTemplate<int>>&>(tpl)
      .UseT2();

  // IWYU: IndirectClass is...*indirect.h
  *dft;
  // IWYU: IndirectClass is...*indirect.h
  *(dft);
}

/**** IWYU_SUMMARY

tests/cxx/template_member_functions.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/template_member_functions.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/template_member_functions.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
#include "tests/cxx/template_member_functions-direct.h"  // for ProvidingTplUsingInDtor, TplGetTplUsingInDtorDefArgProviding

***** IWYU_SUMMARY */
