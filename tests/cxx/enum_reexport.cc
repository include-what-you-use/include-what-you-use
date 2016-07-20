//===--- enum_reexport.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that IWYU ignores uses of enumerators as part of call-expressions.
// The motivation is that unscoped enums are not forward-declarable, so
// the file declaring the function being called must already include
// the enum definition.
//
// This is only strictly true for pre-C++11 unscoped enums, but idiomatically
// they come in two flavors: enums nested in a class and free enums in global
// scope. The logic above is valid for both.

#include "tests/cxx/enum_reexport_func.h"


void TestNestedEnum() {
  FreeFunction(Unscoped::V1);

  Class c;
  c.Method(Unscoped::V2);
}

void TestGlobalEnum() {
  FreeFunction(UE_V1);

  Class c;
  c.Method(UE_V3);
}


/**** IWYU_SUMMARY

(tests/cxx/enum_reexport.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
