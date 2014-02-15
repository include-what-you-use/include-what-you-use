//===--- badinc-extradef.cc - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is to test defining some methods in another translation unit.

#include "tests/cxx/badinc-i2.h"

int I2_Class::AnotherTranslationUnitFn() {
  return 1;
}
int I2_Class::s;


/**** IWYU_SUMMARY

(tests/cxx/badinc-extradef.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
