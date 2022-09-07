//===--- default_tpl_arg.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests handling default type template arguments. In particular, that IWYU
// doesn't crash when they refer to uninstantiated template specializations.

#include "tests/cxx/default_tpl_arg-d1.h"
#include "tests/cxx/default_tpl_arg-d2.h"
#include "tests/cxx/direct.h"

// IWYU: UninstantiatedTpl needs a declaration
// IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
template <typename = UninstantiatedTpl<int>>
struct Tpl {};

template <typename T>
struct Outer1 {
  // IWYU: UninstantiatedTpl needs a declaration
  // IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
  template <typename = UninstantiatedTpl<T>>
  struct Inner {};
};

Outer1<int> o1;

template <typename T1, typename T2>
struct Outer2 {
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  template <typename = IndirectTemplate<T1>>
  struct Inner {};
};

// Test that IWYU should not suggest to provide default template argument
// of an internal template on instantiation side.
// IWYU: IndirectTemplate needs a declaration
Outer2<int, IndirectTemplate<int>> o2;

void Fn() {
  // IWYU: FnWithNonProvidedDefaultTplArg is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  (void)&FnWithNonProvidedDefaultTplArg<>;
  // IWYU: FnWithNonProvidedDefaultTplArg is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  FnWithNonProvidedDefaultTplArg();

  (void)&FnWithProvidedDefaultTplArg<>;
  FnWithProvidedDefaultTplArg();

  // IWYU: IndirectClass is...*indirect.h
  using ProvidingAlias = IndirectClass;
  ProvidingAlias* p = 0;
  // IWYU: NonProvidingAlias is...*default_tpl_arg-i1.h
  NonProvidingAlias* n = 0;

  // IWYU: FnWithNonProvidedDefaultTplArgAndDefaultCallArg is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  FnWithNonProvidedDefaultTplArgAndDefaultCallArg();
  FnWithProvidedDefaultTplArgAndDefaultCallArg1();

  // IWYU: FnWithNonProvidedDefaultTplArgAndDefaultCallArg is...*default_tpl_arg-i1.h
  FnWithNonProvidedDefaultTplArgAndDefaultCallArg(p);
  FnWithProvidedDefaultTplArgAndDefaultCallArg1(p);

  // IWYU: FnWithNonProvidedDefaultTplArgAndDefaultCallArg is...*default_tpl_arg-i1.h
  // IWYU: IndirectClass is...*indirect.h
  FnWithNonProvidedDefaultTplArgAndDefaultCallArg(n);
  // IWYU: IndirectClass is...*indirect.h
  FnWithProvidedDefaultTplArgAndDefaultCallArg1(n);

  // Some additional variations of default arguments. Should not report
  // IndirectClass because it is provided by the templates.
  FnWithProvidedDefaultTplArgAndDefaultCallArg2();
  FnWithProvidedDefaultTplArgAndDefaultCallArg3();
}

/**** IWYU_SUMMARY

tests/cxx/default_tpl_arg.cc should add these lines:
#include "tests/cxx/default_tpl_arg-i1.h"
#include "tests/cxx/indirect.h"

tests/cxx/default_tpl_arg.cc should remove these lines:
- #include "tests/cxx/default_tpl_arg-d1.h"  // lines XX-XX
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/default_tpl_arg.cc:
#include "tests/cxx/default_tpl_arg-d2.h"  // for FnWithProvidedDefaultTplArg, FnWithProvidedDefaultTplArgAndDefaultCallArg1, FnWithProvidedDefaultTplArgAndDefaultCallArg2, FnWithProvidedDefaultTplArgAndDefaultCallArg3
#include "tests/cxx/default_tpl_arg-i1.h"  // for FnWithNonProvidedDefaultTplArg, FnWithNonProvidedDefaultTplArgAndDefaultCallArg, NonProvidingAlias, UninstantiatedTpl
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
