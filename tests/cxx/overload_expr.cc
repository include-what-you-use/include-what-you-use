//===--- overload_expr.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/overload_expr-d?.h" \
//            -Xiwyu --mapping_file=tests/cxx/overload_expr.imp \
//            -I .

// Tests various cases of unresolved lookup expression handling.

#include "tests/cxx/overload_expr-d1.h"
#include "tests/cxx/overload_expr-d2.h"

struct A {};

namespace ns {
struct B {};
}  // namespace ns

int main() {
  TplFn1<int>();
  // IWYU: OverloadedFn is...*overload_expr-i2.h
  TplFn3<A>();
  // IWYU: OverloadedFn2 is...*overload_expr-i2.h
  TplFn4<ns::B>();
}

/**** IWYU_SUMMARY

tests/cxx/overload_expr.cc should add these lines:
#include "tests/cxx/overload_expr-i2.h"

tests/cxx/overload_expr.cc should remove these lines:

The full include-list for tests/cxx/overload_expr.cc:
#include "tests/cxx/overload_expr-d1.h"  // for TplFn1
#include "tests/cxx/overload_expr-d2.h"  // for TplFn3, TplFn4
#include "tests/cxx/overload_expr-i2.h"  // for OverloadedFn, OverloadedFn2

***** IWYU_SUMMARY */
