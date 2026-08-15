//===--- default_tpl_arg-d4.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/default_tpl_arg-d1.h"
#include "tests/cxx/direct.h"

// Some cases from default_tpl_arg.cc have been duplicated here to test them
// with a different provision policy (only template declarations from headers
// can provide their default arguments).

// IWYU: UninstantiatedTpl needs a declaration
// IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
template <typename = UninstantiatedTpl<int>>
struct HeaderTpl {};

template <typename T>
struct HeaderOuter1 {
  // IWYU: UninstantiatedTpl needs a declaration
  // IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
  template <typename = UninstantiatedTpl<T>>
  struct Inner {};
};

inline HeaderOuter1<int> ho1;

template <typename T1, typename T2>
struct HeaderOuter2 {
  // IWYU: IndirectTemplate needs a declaration
  // IWYU: IndirectTemplate is...*indirect.h
  template <typename = IndirectTemplate<T1>>
  struct Inner {};
};

// Test that IWYU should not suggest to provide default template argument
// of an internal template on instantiation side.
// IWYU: IndirectTemplate needs a declaration
inline HeaderOuter2<int, IndirectTemplate<int>> ho2;

// IWYU: IndirectClass is...*indirect.h
using ProvidingAlias = IndirectClass;

/**** IWYU_SUMMARY

tests/cxx/default_tpl_arg-d4.h should add these lines:
#include "tests/cxx/default_tpl_arg-i1.h"
#include "tests/cxx/indirect.h"

tests/cxx/default_tpl_arg-d4.h should remove these lines:
- #include "tests/cxx/default_tpl_arg-d1.h"  // lines XX-XX
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/default_tpl_arg-d4.h:
#include "tests/cxx/default_tpl_arg-i1.h"  // for UninstantiatedTpl
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate

***** IWYU_SUMMARY */
