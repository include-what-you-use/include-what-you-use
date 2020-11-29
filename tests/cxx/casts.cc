//===--- casts.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that we handle correctly handle the various types of cast.

#include "tests/cxx/casts-d1.h"
#include "tests/cxx/casts-d2.h"
#include "tests/cxx/casts-d3.h"

// Prevent regression of a bug where we were ignoring the template
// type arg when we should haven't been, when we were casting to it.
// IWYU: CastsClass needs a declaration
template<typename T> void TestTemplateCastBug(CastsClass* foo) {
  (void)static_cast<T*>(foo);
}

int main() {
  // IWYU: CastsClass needs a declaration
  CastsClass* cc = 0;
  // IWYU: CastsSubclass needs a declaration
  // IWYU: CastsSubclass is...*casts-i1.h
  TestTemplateCastBug<CastsSubclass>(cc);

  // IWYU: CastsI3Convertible is...*casts-i3.h
  CastsI3Convertible ci3;

  // IWYU: CastsI2Base needs a declaration
  // IWYU: CastsI2Derived is...*casts-i2.h
  // IWYU: CastsI3Convertible is...*casts-i3.h
  (void)static_cast<CastsI2Base*>(ci3);
}


/**** IWYU_SUMMARY

tests/cxx/casts.cc should add these lines:
#include "tests/cxx/casts-i1.h"
#include "tests/cxx/casts-i2.h"
#include "tests/cxx/casts-i3.h"

tests/cxx/casts.cc should remove these lines:
- #include "tests/cxx/casts-d1.h"  // lines XX-XX
- #include "tests/cxx/casts-d2.h"  // lines XX-XX
- #include "tests/cxx/casts-d3.h"  // lines XX-XX

The full include-list for tests/cxx/casts.cc:
#include "tests/cxx/casts-i1.h"  // for CastsClass (ptr only), CastsSubclass
#include "tests/cxx/casts-i2.h"  // for CastsI2Base (ptr only), CastsI2Derived
#include "tests/cxx/casts-i3.h"  // for CastsI3Convertible

***** IWYU_SUMMARY */
