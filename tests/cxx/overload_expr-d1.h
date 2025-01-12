//===--- overload_expr-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/overload_expr-i1.h"
#include "tests/cxx/overload_expr-int.h"

// 'overload_expr-int.h' should be kept because it is directly included
// (in contrast to '...-float.h').

template <typename T>
T TplFn1() {
  return ns::add(T{}, T{});
}

template <typename T>
void TplFnReferringVarTpl() {
  // IWYU: var_tpl is...*overload_expr-i4.h
  (void)var_tpl<T>;
}

/**** IWYU_SUMMARY

tests/cxx/overload_expr-d1.h should add these lines:
#include "tests/cxx/overload_expr-i4.h"

tests/cxx/overload_expr-d1.h should remove these lines:
- #include "tests/cxx/overload_expr-i1.h"  // lines XX-XX

The full include-list for tests/cxx/overload_expr-d1.h:
#include "tests/cxx/overload_expr-i4.h"  // for var_tpl
#include "tests/cxx/overload_expr-int.h"  // for add

***** IWYU_SUMMARY */
