//===--- implicit_ctor.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --check_also="tests/cxx/*-d1.h" -I .

// If you define an API that accepts an argument of class type or
// const reference to a class type with an implicit constructor which takes
// exactly one argument, you must provide the definition for the class.
//
// If you use an API and that use triggers such an implicit constructor
// call, you are *not* considered to have used that constructor,
// relying on the previous rule to make the code complete definition
// available. This is so called "autocast".
//
// In other cases of implicit construction (without specifying class name
// explicitly), the complete class definition needs to be available.
//
// This tests that logic.

#include "tests/cxx/implicit_ctor-d1.h"
#include "tests/cxx/implicit_ctor-d2.h"

// No reporting types for "autocast" for .cpp-file-local functions...
// IWYU: IndirectWithImplicitCtor needs a declaration
static int LocalFn(const IndirectWithImplicitCtor&) {
  return 1;
}

// ... or for .cpp-file-definitions of functions declared in a header.
// IWYU: IndirectWithImplicitCtor needs a declaration
int ImplicitCtorRefFn(const IndirectWithImplicitCtor&) {
  return 2;
}

// We don't need IndirectWithImplicitCtor even though we must convert to it.
int a = ImplicitCtorFn(1);
int b = ImplicitCtorRefFn(2);

// But we do need it if we do the conversion explicitly.
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
int c = ImplicitCtorFn(IndirectWithImplicitCtor(3));
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
int d = ImplicitCtorRefFn(IndirectWithImplicitCtor(4));

// LocalFn doesn't provide IndirectWithImplicitCtor type info.
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
int e = LocalFn(5);
// InlineImplicitCtorRefFn should provide parameter type info,
// hence no reporting.
int f = InlineImplicitCtorRefFn(6);

// Make sure we are responsible for the conversion when it's not for a
// function call.
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
IndirectWithImplicitCtor indirect
    // IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
    = 1;

// IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
using Providing = NoAutocastCtor;
// IWYU: NoTrivialCtorDtor is...*implicit_ctor-i2.h
using NoTrivialCtorDtorProvidingAlias = NoTrivialCtorDtor;

// IWYU: NoAutocastCtor needs a declaration
void FnTakingDirectly(NoAutocastCtor);
void FnTakingByProvidingAlias(Providing);
void FnTakingByNonProvidingAlias(NonProviding);

template <typename T>
void TplFn(T t);

// IWYU: NoAutocastCtor needs a declaration
void TestNonAutocastConstruction(const NoAutocastCtor& par) {
  // IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
  FnTakingDirectly(par);
  FnTakingByProvidingAlias(par);
  // IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
  FnTakingByNonProvidingAlias(par);

  Providing* p = nullptr;
  NonProviding* n = nullptr;
  FnTakingDirectly(*p);
  // IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
  FnTakingDirectly(*n);

  // IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
  TplFn(par);
  TplFn<Providing>(par);
  // IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
  TplFn<NonProviding>(par);

  // Test handling presence of intermediate 'CXXBindTemporaryExpr' node
  // in the AST.

  // IWYU: NoTrivialCtorDtor needs a declaration
  NoTrivialCtorDtor* nt = nullptr;
  TplFn<NoTrivialCtorDtorProvidingAlias>(*nt);
  // IWYU: NoTrivialCtorDtor is...*implicit_ctor-i2.h
  TplFn<NoTrivialCtorDtorNonProvidingAlias>(*nt);

  // IWYU: NoAutocastCtor needs a declaration
  // IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
  const NoAutocastCtor& x1 = {};
  // IWYU: NoAutocastCtor needs a declaration
  // IWYU: NoAutocastCtor is...*implicit_ctor-i2.h
  const NoAutocastCtor& x2 = {3, 4};
}

struct NonAggregate {
  NonAggregate();
};

void OverloadedFn(int, NonAggregate);
void OverloadedFn(float, NonAggregate);

template <typename T>
void TestNoCrashOnUnresolvedCall(T t) {
  OverloadedFn(t, NonAggregate{});
  t.SomeFn(NonAggregate{});
}

/**** IWYU_SUMMARY

tests/cxx/implicit_ctor.cc should add these lines:
#include "tests/cxx/implicit_ctor-i2.h"

tests/cxx/implicit_ctor.cc should remove these lines:

The full include-list for tests/cxx/implicit_ctor.cc:
#include "tests/cxx/implicit_ctor-d1.h"  // for ImplicitCtorFn, ImplicitCtorRefFn, InlineImplicitCtorRefFn
#include "tests/cxx/implicit_ctor-d2.h"  // for NoTrivialCtorDtorNonProvidingAlias, NonProviding
#include "tests/cxx/implicit_ctor-i2.h"  // for IndirectWithImplicitCtor, NoAutocastCtor, NoTrivialCtorDtor

***** IWYU_SUMMARY */
