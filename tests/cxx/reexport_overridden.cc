//===--- reexport_overridden.cc - test input file for iwyu ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests handling overriding method signatures.

#include "tests/cxx/direct.h"
#include "tests/cxx/reexport_overridden-d1.h"
#include "tests/cxx/reexport_overridden-d2.h"

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
#include "tests/cxx/reexport_overridden-i2.h"

tests/cxx/reexport_overridden.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/reexport_overridden-d2.h"  // lines XX-XX

The full include-list for tests/cxx/reexport_overridden.cc:
#include "tests/cxx/reexport_overridden-d1.h"  // for Derived2
#include "tests/cxx/reexport_overridden-i2.h"  // for FwdAutoconvParam, FwdRetType

***** IWYU_SUMMARY */
