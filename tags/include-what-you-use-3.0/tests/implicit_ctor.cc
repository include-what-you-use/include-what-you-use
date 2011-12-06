//===--- implicit_ctor.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// If you define an API that accepts an argument of class type or
// const reference to a class type with an implicit constructor, you
// must provide the definition for the class.
//
// If you use an API and that use triggers an implicit constructor
// call, you are *not* considered to have used that constructor,
// relying on the previous rule to make the code complete definition
// available.
//
// This tests that logic.

#include "tests/implicit_ctor-d1.h"

// We don't need IndirectWithImplicitCtor even though we must convert to it.
int a = ImplicitCtorFn(1);
int b = ImplicitCtorRefFn(2);

// But we do need it if we do the conversion explicitly.
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
int c = ImplicitCtorFn(IndirectWithImplicitCtor(3));
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
int d = ImplicitCtorRefFn(IndirectWithImplicitCtor(4));

// Make sure we are responsible for the conversion when it's not for a
// function call.
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
IndirectWithImplicitCtor indirect
    // IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
    = 1;


/**** IWYU_SUMMARY

tests/implicit_ctor.cc should add these lines:
#include "tests/implicit_ctor-i2.h"

tests/implicit_ctor.cc should remove these lines:

The full include-list for tests/implicit_ctor.cc:
#include "tests/implicit_ctor-d1.h"  // for ImplicitCtorFn, ImplicitCtorRefFn
#include "tests/implicit_ctor-i2.h"  // for IndirectWithImplicitCtor

***** IWYU_SUMMARY */
