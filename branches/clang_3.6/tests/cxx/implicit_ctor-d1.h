//===--- implicit_ctor-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

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

/**** IWYU_SUMMARY

tests/cxx/implicit_ctor-d1.h should add these lines:
#include "tests/cxx/implicit_ctor-i2.h"

tests/cxx/implicit_ctor-d1.h should remove these lines:
- #include "tests/cxx/implicit_ctor-i1.h"  // lines XX-XX

The full include-list for tests/cxx/implicit_ctor-d1.h:
#include "tests/cxx/implicit_ctor-i2.h"  // for IndirectWithImplicitCtor

***** IWYU_SUMMARY */
