//===--- casts.cc - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that we handle correctly handle the various types of cast.

#include "tests/casts-d1.h"

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
}


/**** IWYU_SUMMARY

tests/casts.cc should add these lines:
#include "tests/casts-i1.h"

tests/casts.cc should remove these lines:
- #include "tests/casts-d1.h"  // lines XX-XX

The full include-list for tests/casts.cc:
#include "tests/casts-i1.h"  // for CastsClass (ptr only), CastsSubclass

***** IWYU_SUMMARY */
