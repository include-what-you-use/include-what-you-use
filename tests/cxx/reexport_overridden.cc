//===--- reexport_overridden.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU has a special rule for overridden method declarations: any types in the
// signature should be ignored for IWYU's purposes, because the derived
// declaration must include the base header, and thereby all its types.
//
// However, *other* uses, such as in the method definition (both signature and
// body), still need to fend for themselves.

#include "tests/cxx/reexport_overridden.h"
#include "tests/cxx/direct.h"
#include "tests/cxx/reexport_overridden-d2.h"

// IndirectClass is reexported from Base to the Derived
IndirectClass Derived::GetIndirect() {
  // IWYU: IndirectClass is...*indirect.h
  return IndirectClass();
}

// Similarly for FwdRetType -- Base reexports its forward declarations
FwdRetType Derived::ReturnType() {
  // IWYU: FwdRetType is...*reexport_overridden-indirect.h
  return FwdRetType();
}

// Even if the return type is not mentioned in the body, the compiler requires
// the complete type for the method definition.

// Simulate abort().
__attribute__((__noreturn__)) void abort();

FwdRetType Derived::ReturnTypeUnusedInBody() {
  abort();
}

// No diagnostic expected for unused arguments.
void Derived::ArgumentUnused(const IndirectClass&) {
}

/**** IWYU_SUMMARY

tests/cxx/reexport_overridden.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/reexport_overridden-indirect.h"

tests/cxx/reexport_overridden.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/reexport_overridden-d2.h"  // lines XX-XX

The full include-list for tests/cxx/reexport_overridden.cc:
#include "tests/cxx/reexport_overridden.h"
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/reexport_overridden-indirect.h"  // for FwdRetType

***** IWYU_SUMMARY */
