//===--- overload_expr-d2.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/overload_expr-i1.h"

// IWYU should report any of the files declaring 'ns::add' because it is
// a qualified name.

template <typename T>
T TplFn2() {
  // IWYU: ns::add is...*overload_expr-int.h
  return ns::add(T{}, T{});
}

template <typename T>
void TplFn3() {
  // It is allowed not to include any definition of 'OverloadedFn' at this
  // point.
  return OverloadedFn(T{});
}

/**** IWYU_SUMMARY

tests/cxx/overload_expr-d2.h should add these lines:
#include "tests/cxx/overload_expr-int.h"

tests/cxx/overload_expr-d2.h should remove these lines:
- #include "tests/cxx/overload_expr-i1.h"  // lines XX-XX

The full include-list for tests/cxx/overload_expr-d2.h:
#include "tests/cxx/overload_expr-int.h"  // for add

***** IWYU_SUMMARY */
