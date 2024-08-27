//===--- reexport_overridden.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -Xiwyu --check_also=tests/cxx/reexport_overridden-d1.h

// Tests handling overriding method signatures.

#include "tests/cxx/direct.h"
#include "tests/cxx/reexport_overridden-d1.h"
#include "tests/cxx/reexport_overridden-d2.h"

// TODO(bolshakov): it is questionable whether types provided by the primary
// declaration should be reported from an implementation.
// IWYU: IndirectClass is...*indirect.h
IndirectClass Derived1::GetIndirect() {
  // No need to forward-declare a type used in the method signature.
  IndirectClass* pi = nullptr;
  // Other types shoud be reported.
  // IWYU: FwdRetType needs a declaration
  FwdRetType* pf = nullptr;
  // IWYU: IndirectClass is...*indirect.h
  return IndirectClass();
}

// Base reexports only the forward declaration of FwdRetType.
// IWYU: FwdRetType is...*reexport_overridden-i2.h
FwdRetType Derived1::ReturnType() {
  // No need to forward-declare here.
  FwdRetType* p = nullptr;
  // IWYU: FwdRetType is...*reexport_overridden-i2.h
  return FwdRetType();
}

// Even if the return type is not mentioned in the body, the compiler requires
// the complete type for the method definition.

// IWYU: FwdRetType is...*reexport_overridden-i2.h
FwdRetType Derived1::ReturnTypeUnusedInBody() {
  throw 1;
}

// Info: ReturnTemplate definition forces instantiation of the template
// specialization which may affect the IWYU analysis of the method declaration.
// IWYU: IndirectTemplate is...*indirect.h
// IWYU: FwdRetType is...*reexport_overridden-i2.h
IndirectTemplate<FwdRetType> Derived1::ReturnTemplate() {
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: FwdRetType is...*reexport_overridden-i2.h
  return {};
}

// No diagnostic expected for unused arguments.
void Derived1::ArgumentUnused(const IndirectClass&) {
  // The full use should still be reported.
  // IWYU: IndirectClass is...*indirect.h
  IndirectClass ic;
}

// IWYU: Alias is...*reexport_overridden-i3.h
// IWYU: IndirectClass is...*indirect.h
void Derived1::TakeNoConvParam(Alias arg) {
  // No need to forward-declare IndirectClass.
  IndirectClass* p = &arg;
}

// Info: taking the template specialization by value forces its instantiation
// at the method definition which may affect the IWYU analysis of the method
// declaration.
// IWYU: IndirectTemplate is...*indirect.h
// IWYU: IndirectClass is...*indirect.h
void Derived1::TakeTemplate(IndirectTemplate<IndirectClass>) {
}

// IWYU: TemplatePtrAlias is...*reexport_overridden-i3.h
void Derived1::TakeAliasedTemplatePtr(TemplatePtrAlias p1) {
  // No need to forward-declare IndirectTemplate or IndirectClass.
  IndirectTemplate<IndirectClass>* p2 = p1;
  IndirectClass* p = nullptr;
}

void Fn() {
  // Test that IWYU uses the most basic method declarations for
  // the author-intent analysis. None of the used types are forward-declared
  // in the file defining Derived1 and Derived2, hence they might be erroneously
  // considered as (if should be) provided, but the Base provides only
  // the GetIndirect return type.
  Derived2 d;
  d.GetIndirect();
  // IWYU: FwdRetType is...*reexport_overridden-i2.h
  d.ReturnType();
  // IWYU: FwdAutoconvParam is...*reexport_overridden-i2.h
  d.TakeFwdAutoconvParam(1);
}

/**** IWYU_SUMMARY

tests/cxx/reexport_overridden.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/reexport_overridden-i2.h"
#include "tests/cxx/reexport_overridden-i3.h"

tests/cxx/reexport_overridden.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/reexport_overridden-d2.h"  // lines XX-XX

The full include-list for tests/cxx/reexport_overridden.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
#include "tests/cxx/reexport_overridden-d1.h"  // for Derived1, Derived2
#include "tests/cxx/reexport_overridden-i2.h"  // for FwdAutoconvParam, FwdRetType
#include "tests/cxx/reexport_overridden-i3.h"  // for Alias, TemplatePtrAlias

***** IWYU_SUMMARY */
