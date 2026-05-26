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

int g = ImplicitCtorInPartialFn(7);

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

  // Test that IWYU finds the definition among redeclarations which contains
  // the implicit ctor and doesn't require the complete type here.
  TakeMultipleRedeclStruct(1);
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

// IWYU: OuterAggregate1 needs a declaration
void TakeOuterAggregate1(OuterAggregate1);
// IWYU: OuterAggregate2 needs a declaration
void TakeOuterAggregate2(OuterAggregate2);
// IWYU: OuterAggregateWithRef needs a declaration
void TakeOuterAggregateWithRef(OuterAggregateWithRef);
void TakeNonProvidingOuterAggregate1(NonProvidingOuterAggregate1);
void TakeProvidingOuterAggregate1(ProvidingOuterAggregate1);
// IWYU: AggregateWithNonAggregate needs a declaration
void TakeAggregateWithNonAggregate(AggregateWithNonAggregate);

template <typename T, typename>
void TestAggregateInitTplFn() {
  void TakeT(T);
  TakeT({});
}

// IWYU: OuterAggregate1 needs a declaration
void TestAggregateInit(const OuterAggregate1& par) {
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  auto local_copy = par;
  // IWYU: OuterAggregate1 needs a declaration
  const OuterAggregate1& local_ref{par};
  // IWYU: OuterAggregate1 needs a declaration
  const OuterAggregate1&& rvalue_ref{static_cast<const OuterAggregate1&&>(par)};
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  TakeOuterAggregate1(par);
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  TakeOuterAggregate1({});
  // Requiring InnerAggregate1 complete type here facilitates probable
  // transforming the type of 'inner' into 'const InnerAggregate1&'.
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  // IWYU: InnerAggregate1 is...*implicit_ctor-i2.h
  TakeOuterAggregate1({{}});
  // IWYU: OuterAggregate1 needs a declaration
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  const OuterAggregate1& oa11 = {};
  // IWYU: OuterAggregate1 needs a declaration
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  // IWYU: InnerAggregate1 is...*implicit_ctor-i2.h
  const OuterAggregate1& oa12 = {{}};
  // IWYU: OuterAggregate2 is...*implicit_ctor-i2.h
  TakeOuterAggregate2({1, 2, 3, 4});
  // IWYU: OuterAggregate2 is...*implicit_ctor-i2.h
  // IWYU: InnerAggregate2 is...*implicit_ctor-i2.h
  TakeOuterAggregate2({1, {2, 3}, 4});
  // IWYU: OuterAggregateWithRef is...*implicit_ctor-i2.h
  // IWYU: InnerAggregate1 is...*implicit_ctor-i2.h
  TakeOuterAggregateWithRef({{}});
  // IWYU: OuterAggregate1 needs a declaration
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  // IWYU: InnerAggregate1 needs a declaration
  TestAggregateInitTplFn<OuterAggregate1, InnerAggregate1>();
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  const NonProvidingOuterAggregate1& npoa1 = {};
  const ProvidingOuterAggregate1& poa1 = {};
  // IWYU: OuterAggregate1 is...*implicit_ctor-i2.h
  TakeNonProvidingOuterAggregate1({});
  TakeProvidingOuterAggregate1({});

  // There is no point in reporting I2NonAggregate here.
  // IWYU: AggregateWithNonAggregate needs a declaration
  // IWYU: AggregateWithNonAggregate is...*implicit_ctor-i2.h
  const AggregateWithNonAggregate& awna = {};
  // IWYU: AggregateWithNonAggregate is...*implicit_ctor-i2.h
  TakeAggregateWithNonAggregate({});
  // In these cases, I2NonAggregate should be reported because
  // AggregateWithNonAggregate::na could be 'const I2NonAggregate&'.
  // IWYU: I2NonAggregate is...*implicit_ctor-i2.h
  // IWYU: AggregateWithNonAggregate is...*implicit_ctor-i2.h
  TakeAggregateWithNonAggregate({{}});
  // IWYU: I2NonAggregate is...*implicit_ctor-i2.h
  // IWYU: AggregateWithNonAggregate is...*implicit_ctor-i2.h
  TakeAggregateWithNonAggregate({{1}});
  // In contrast to subaggregates, AggregateWithNonAggregate::na could be
  // a reference even in this case.
  // IWYU: I2NonAggregate is...*implicit_ctor-i2.h
  // IWYU: AggregateWithNonAggregate is...*implicit_ctor-i2.h
  TakeAggregateWithNonAggregate({1});
}

/**** IWYU_SUMMARY

tests/cxx/implicit_ctor.cc should add these lines:
#include "tests/cxx/implicit_ctor-i2.h"

tests/cxx/implicit_ctor.cc should remove these lines:

The full include-list for tests/cxx/implicit_ctor.cc:
#include "tests/cxx/implicit_ctor-d1.h"  // for ImplicitCtorFn, ImplicitCtorInPartialFn, ImplicitCtorRefFn, InlineImplicitCtorRefFn, ProvidingOuterAggregate1, TakeMultipleRedeclStruct
#include "tests/cxx/implicit_ctor-d2.h"  // for NoTrivialCtorDtorNonProvidingAlias, NonProviding, NonProvidingOuterAggregate1
#include "tests/cxx/implicit_ctor-i2.h"  // for AggregateWithNonAggregate, I2NonAggregate, IndirectWithImplicitCtor, InnerAggregate1, InnerAggregate2, NoAutocastCtor, NoTrivialCtorDtor, OuterAggregate1, OuterAggregate2, OuterAggregateWithRef

***** IWYU_SUMMARY */
