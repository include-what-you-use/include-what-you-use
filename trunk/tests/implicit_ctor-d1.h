//===--- implicit_ctor-d1.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/implicit_ctor-i1.h"

// We do need IndirectWithImplicitCtor even though we just take a
// reference to it.  This is because the class has an implicit
// constructor.

// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
int ImplicitCtorFn(IndirectWithImplicitCtor) {
  return 1;
}

// IWYU: IndirectWithImplicitCtor needs a declaration
// IWYU: IndirectWithImplicitCtor is...*implicit_ctor-i2.h
int ImplicitCtorRefFn(const IndirectWithImplicitCtor&) {
  return 2;
}

/**** IWYU_SUMMARY

tests/implicit_ctor-d1.h should add these lines:
#include "tests/implicit_ctor-i2.h"

tests/implicit_ctor-d1.h should remove these lines:
- #include "tests/implicit_ctor-i1.h"  // lines XX-XX

The full include-list for tests/implicit_ctor-d1.h:
#include "tests/implicit_ctor-i2.h"  // for IndirectWithImplicitCtor

***** IWYU_SUMMARY */
