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

// IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
template <typename = UninstantiatedTpl<int>>
struct Tpl {};

template <typename T>
struct Outer {
  // IWYU: UninstantiatedTpl is...*default_tpl_arg-i1.h
  template <typename = UninstantiatedTpl<T>>
  struct Inner {};
};

Outer<int> o;

/**** IWYU_SUMMARY

tests/cxx/default_tpl_arg.cc should add these lines:
#include "tests/cxx/default_tpl_arg-i1.h"

tests/cxx/default_tpl_arg.cc should remove these lines:
- #include "tests/cxx/default_tpl_arg-d1.h"  // lines XX-XX

The full include-list for tests/cxx/default_tpl_arg.cc:
#include "tests/cxx/default_tpl_arg-i1.h"  // for UninstantiatedTpl

***** IWYU_SUMMARY */
