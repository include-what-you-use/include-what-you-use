//===--- member_expr.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that we correctly detect iwyu use with member accesses.

#include "tests/cxx/member_expr-d1.h"
#include "tests/cxx/direct.h"

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


// Because any member expression instantiates the template, the template
// specialization used as a member expression base requires full type info for
// all the template type arguments that are full-type-used for member
// initializers, non-static data members or nested typedef instantiation,
// regardless of whether those types are part of the member expression or not.

// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectClass needs a declaration
int RefFn(const IndirectTemplate<IndirectClass>& ic) {
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  ic.Method();
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  return ic.a;
}

// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectClass needs a declaration
int PtrFn(const IndirectTemplate<IndirectClass>* ic) {
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  ic->Method();
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  return ic->a;
}

void TemplateStaticFn() {
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass needs a declaration
  // IWYU: IndirectClass is...*indirect.h
  IndirectTemplate<IndirectClass>::StaticMethod();
}

// IWYU: IndirectTemplate needs a declaration
// IWYU: IndirectClass needs a declaration
void ViaMacro(const IndirectTemplate<IndirectClass>& ic) {
  // We should figure out we need IndirectTemplate and IndirectClass
  // because of the 'ic.', even if the member-expr itself is in another file
  // due to the macro.
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  ic.CALL_METHOD;

  // Likewise, we 'own' this member expr because we own the dot.
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  IC.Method();
  // IWYU: IndirectTemplate is...*indirect.h
  // IWYU: IndirectClass is...*indirect.h
  IC.CALL_METHOD;

  IC.
      // IWYU: IndirectTemplate is...*indirect.h
      // IWYU: IndirectClass is...*indirect.h
      CALL_METHOD;

  // But this member-expr is entirely in the macro, so we don't own it.
  IC_CALL_METHOD;
}

// Test type use with member expression inside a template.

template <typename T>
void MemberExprInside() {
  T* t = nullptr;
  (*t)->Method();
}

template <typename T>
void MemberExprWithDerefInside() {
  T t;
  auto& r = *t;
  // A type after dereferencing t should be in the resugaring map in order
  // to be reported.
  r.Method();
}

class IndirectClass;  // IndirectClassPtr doesn't provide IndirectClass.
typedef IndirectClass* IndirectClassPtr;

namespace ns {
using ::IndirectClassPtr;
}

void Fn() {
  // IndirectClass is hidden by a pointer and (at least) two levels of sugar.
  // IWYU: IndirectClass is...*indirect.h
  MemberExprInside<ns::IndirectClassPtr>();
  // IWYU: IndirectClass is...*indirect.h
  MemberExprWithDerefInside<IndirectClassPtr>();
}

/**** IWYU_SUMMARY

tests/cxx/member_expr.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/member_expr.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- class IndirectClass;  // lines XX-XX

The full include-list for tests/cxx/member_expr.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass, IndirectTemplate
#include "tests/cxx/member_expr-d1.h"  // for CALL_METHOD, IC, IC_CALL_METHOD

***** IWYU_SUMMARY */
