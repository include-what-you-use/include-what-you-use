//===--- typedefs_and_resugaring.cc - test input file for iwyu ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that when we have a typedef where the author disclaims
// responsibility (so the user has to #include the underlying type),
// we don't run into problems in a template context where the
// underlying type gets resugared back to the original type.
//
// In other words, this protects against regressions involving the
// following bug we had in iwyu:
//
// ReportTypeUse was reporting on the use of a typedef type, then on
// the use of the underlying type (since it decided the typedef-user
// was responsible for the underlying type).  The typedef type was a
// template parameter, though, so for the underlying type, iwyu was
// doing type-resugaring and getting back to the same original type
// that we were trying to recurse on!  That is, TypedefType -> recurse
// on UnderlyingType -> resugared to TypedefType -> recurse on
// UnderlyingType -> infinite loop.

#include "tests/cxx/direct.h"
#include "tests/cxx/typedefs_and_resugaring-d1.h"

template <typename T> void TplFn() {
  T t;
  (void)t;
}

int main() {
  // TplFn uses MyTypedef.  We are also responsible for IndirectClass
  // since the MyTypedef author decided not to be.  This tests we
  // don't get into an infinite loop where we go to mark the need for
  // IndirectClass, but it gets resugared back to MyTypedef (which is
  // the template arg).
  // IWYU: IndirectClass is...*indirect.h
  TplFn<MyTypedef>();
}

/**** IWYU_SUMMARY

tests/cxx/typedefs_and_resugaring.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/typedefs_and_resugaring.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/typedefs_and_resugaring.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/typedefs_and_resugaring-d1.h"  // for MyTypedef

***** IWYU_SUMMARY */
