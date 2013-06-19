//===--- member_expr.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we correctly detect iwyu use with member accesses.

#include "tests/member_expr-d1.h"
#include "tests/direct.h"

// IWYU: IndirectClass needs a declaration
int RefFn(const IndirectClass& ic) {
  // IWYU: IndirectClass is...*indirect.h
  ic.Method();
  // IWYU: IndirectClass is...*indirect.h
  return ic.a;
}

// IWYU: IndirectClass needs a declaration
int PtrFn(const IndirectClass* ic) {
  // IWYU: IndirectClass is...*indirect.h
  ic->Method();
  // IWYU: IndirectClass is...*indirect.h
  return ic->a;
}

void StaticFn() {
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass::StaticMethod();
}

// IWYU: IndirectClass needs a declaration
void ViaMacro(const IndirectClass& ic) {
  // We should figure out we need IndirectClass because of the 'ic.',
  // even if the member-expr itself is in another file due to the macro.
  // IWYU: IndirectClass is...*indirect.h
  ic.CALL_METHOD;

  // Likewise, we 'own' this member expr because we own the dot.
  // IWYU: IndirectClass is...*indirect.h
  IC.Method();
  // IWYU: IndirectClass is...*indirect.h
  IC.CALL_METHOD;

  IC
      .
      // IWYU: IndirectClass is...*indirect.h
      CALL_METHOD;

  // But this member-expr is entirely in the macro, so we don't own it.
  IC_CALL_METHOD;
}


/**** IWYU_SUMMARY

tests/member_expr.cc should add these lines:
#include "tests/indirect.h"

tests/member_expr.cc should remove these lines:
- #include "tests/direct.h"  // lines XX-XX

The full include-list for tests/member_expr.cc:
#include "tests/indirect.h"  // for IndirectClass
#include "tests/member_expr-d1.h"  // for CALL_METHOD, IC, IC_CALL_METHOD

***** IWYU_SUMMARY */
