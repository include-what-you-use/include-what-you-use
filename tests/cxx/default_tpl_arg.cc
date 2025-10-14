//===--- default_tpl_arg.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --check_also="tests/cxx/*-d2.h"

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

template <typename = int, typename>
// IWYU: ClassTpl is...*default_tpl_arg-i1.h
class ClassTpl;

// Test that IWYU doesn't remap the required declaration to the definition and
// hence reports it here.
template <typename = int, typename>
// IWYU: ClassTplWithDefinition is...*default_tpl_arg-i1.h
class ClassTplWithDefinition {};

template <typename = int, typename>
// IWYU: VarTpl is...*default_tpl_arg-i1.h
extern int VarTpl;

// No need to report anything here.
template <typename, typename>
using AliasTpl1 = int;
template <int, int>
using AliasTpl2 = int;
template <template <int> typename, template <int> typename>
using AliasTpl3 = int;

template <typename = int, typename>
// IWYU: AliasTpl1 is...*default_tpl_arg-i1.h
using AliasTpl1 = int;

template <typename = int, typename>
void FnTpl();

template <int = 0, int>
// IWYU: AliasTpl2 is...*default_tpl_arg-i1.h
using AliasTpl2 = int;

// IWYU: SomeTpl is...*default_tpl_arg-i1.h
template <template <int> typename = SomeTpl, template <int> typename>
// IWYU: AliasTpl3 is...*default_tpl_arg-i1.h
using AliasTpl3 = int;

template <typename = int, int>
// IWYU: AliasTpl4 is...*default_tpl_arg-i1.h
using AliasTpl4 = int;

// The needed redeclaration is in a directly included file.
template <int = 0, int, int>
using AliasTpl5 = int;

// This redeclaration doesn't require any previous declaration to be present.
template <int, int = 0>
using AliasTpl6 = int;

// The required redeclaration is the previous one.
template <int = 0, int>
using AliasTpl6 = int;

// Just to avoid suggestion to remove unused forward-declaration.
ClassTpl<>* class_tpl_ptr;

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
#include "tests/cxx/default_tpl_arg-d2.h"  // for AliasTpl5, FnWithProvidedDefaultTplArg, FnWithProvidedDefaultTplArgAndDefaultCallArg1, FnWithProvidedDefaultTplArgAndDefaultCallArg2, FnWithProvidedDefaultTplArgAndDefaultCallArg3
#include "tests/cxx/default_tpl_arg-i1.h"  // for AliasTpl1, AliasTpl2, AliasTpl3, AliasTpl4, ClassTpl, ClassTplWithDefinition, FnWithNonProvidedDefaultTplArg, FnWithNonProvidedDefaultTplArgAndDefaultCallArg, NonProvidingAlias, SomeTpl, UninstantiatedTpl, VarTpl
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
template <typename = int, typename> class ClassTpl;  // lines XX-XX+2

***** IWYU_SUMMARY */
