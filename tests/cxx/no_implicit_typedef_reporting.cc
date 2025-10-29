//===--- no_implicit_typedef_reporting.cc - test input file for iwyu ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . \
//            -Xiwyu --check_also="tests/cxx/no_implicit_typedef_reporting-d2.h"

// Tests that IWYU doesn't suggest to include typedef-containing header due to
// implicit-only use of the typedef, i.e. when it is not explicitly written
// in a source.

#include "tests/cxx/no_implicit_typedef_reporting-d1.h"
#include "tests/cxx/no_implicit_typedef_reporting-d2.h"

template <class T1, class T2>
struct Template {
  T1 t1;
  decltype(T2::typedefed) t2;
};

void Fn() {
  // Test use in unary expression.
  (void)sizeof(Struct::typedefed);

  Struct s;
  // Test use for pointer arithmetic.
  void* p = &s.typedefed + 10;

  // Test use in instantiated template.
  Template<decltype(Struct::typedefed), Struct> tpl;

  // Test use in a lambda capture.
  [i = s.typedefed] {}();
}

/**** IWYU_SUMMARY

tests/cxx/no_implicit_typedef_reporting.cc should add these lines:

tests/cxx/no_implicit_typedef_reporting.cc should remove these lines:
- #include "tests/cxx/no_implicit_typedef_reporting-d1.h"  // lines XX-XX

The full include-list for tests/cxx/no_implicit_typedef_reporting.cc:
#include "tests/cxx/no_implicit_typedef_reporting-d2.h"  // for Struct

***** IWYU_SUMMARY */
