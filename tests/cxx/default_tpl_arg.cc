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

/**** IWYU_SUMMARY

tests/cxx/default_tpl_arg.cc should add these lines:
#include "tests/cxx/default_tpl_arg-i1.h"
#include "tests/cxx/indirect.h"

tests/cxx/default_tpl_arg.cc should remove these lines:
- #include "tests/cxx/default_tpl_arg-d1.h"  // lines XX-XX
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/default_tpl_arg.cc:
#include "tests/cxx/default_tpl_arg-i1.h"  // for UninstantiatedTpl
#include "tests/cxx/indirect.h"  // for IndirectTemplate

***** IWYU_SUMMARY */
