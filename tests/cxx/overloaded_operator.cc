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
  // Test that the extra parentheses doesn't hide the type as-written, and IWYU
  // doesn't suggest the underlying type, again.
  (void)((t3) == t2);
}

/**** IWYU_SUMMARY

(tests/cxx/overloaded_operator.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
