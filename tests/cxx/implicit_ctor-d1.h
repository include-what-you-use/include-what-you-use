//===--- implicit_ctor-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/direct.h"
#include "tests/cxx/implicit_ctor-i1.h"

// We do need IndirectWithImplicitCtor even though we just take a
// reference to it.  This is because the class has an implicit
// constructor.

// IWYU: IndirectWithImplicitCtor needs a declaration
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h.*for autocast
int ImplicitCtorFn(IndirectWithImplicitCtor);

// IWYU: IndirectWithImplicitCtor needs a declaration
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h.*for autocast
int ImplicitCtorRefFn(const IndirectWithImplicitCtor&);

// Reporting types for "autocast" for header-defined functions still makes sense
// as opposed to function definitions in source files.
// IWYU: IndirectWithImplicitCtor needs a declaration
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h.*for autocast
inline int InlineImplicitCtorRefFn(const IndirectWithImplicitCtor&) {
  return 1;
}

// Test parameter type uses that do not require special handling for "autocast".
int NoAutocastFn(
    // A subtle c++ point: forward-declaring is ok for nonconst, because
    // you can't do implicit conversion to a non-const reference
    // (implicit conversion involves creating a temporary, which
    // doesn't bind to non-const references).
    // IWYU: IndirectWithImplicitCtor needs a declaration
    IndirectWithImplicitCtor& nonconst,
    // Forward-declaring is ok because ptrref is a const reference
    // to a *pointer*.
    // IWYU: IndirectWithImplicitCtor needs a declaration
    IndirectWithImplicitCtor* const& ptrref,
    // IWYU: IndirectClass needs a declaration
    const IndirectClass&,
    // IWYU: IndirectClass needs a declaration
    IndirectClass);

/**** IWYU_SUMMARY

tests/cxx/implicit_ctor-d1.h should add these lines:
#include "tests/cxx/implicit_ctor-i2.h"
class IndirectClass;

tests/cxx/implicit_ctor-d1.h should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/implicit_ctor-i1.h"  // lines XX-XX

The full include-list for tests/cxx/implicit_ctor-d1.h:
#include "tests/cxx/implicit_ctor-i2.h"  // for IndirectWithImplicitCtor
class IndirectClass;

***** IWYU_SUMMARY */
