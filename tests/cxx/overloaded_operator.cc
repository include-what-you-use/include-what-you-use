//===--- overloaded_operator.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests some cases related to user-defined operators.

#include "tests/cxx/overloaded_operator-direct.h"

void Fn() {
  ProvidingAlias t1, t2;
  // Test that IWYU determines the first argument type correctly
  // as the providing alias and doesn't suggest the underlying type.
  (void)(t1 == t2);
  ProvidingRefAlias t3 = t1;
  // Test that the extra parentheses don't hide the type as-written, and IWYU
  // doesn't suggest the underlying type, again.
  (void)((t3) == t2);

  // IWYU: Derived is...*indirect.h
  Derived d1, d2;
  // Test that IWYU ignores derived-to-base implicit conversion and reports
  // the derived type, not the base.
  // IWYU: Derived is...*indirect.h
  (void)(d1 == d2);
}

/**** IWYU_SUMMARY

tests/cxx/overloaded_operator.cc should add these lines:
#include "tests/cxx/overloaded_operator-indirect.h"

tests/cxx/overloaded_operator.cc should remove these lines:

The full include-list for tests/cxx/overloaded_operator.cc:
#include "tests/cxx/overloaded_operator-direct.h"  // for ProvidingAlias, ProvidingRefAlias
#include "tests/cxx/overloaded_operator-indirect.h"  // for Derived

***** IWYU_SUMMARY */
