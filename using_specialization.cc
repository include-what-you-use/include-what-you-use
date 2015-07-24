//===--- using_specialization.cc - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

namespace ns {
  #include "template_specialization-i1.h"
  #include "template_specialization-i2.h"
}

void use_non_specialization() {
  using ns::Foo;
  Foo<float> f;
}

/**** IWYU_SUMMARY

tests/cxx/using_specialization.cc should add these lines:

tests/cxx/using_specialization.cc should remove these lines:
- #include "template_specialization-i2.h"  // lines XX-XX

The full include-list for tests/cxx/using_specialization.cc:
#include "template_specialization-i1.h"  // for Foo

***** IWYU_SUMMARY */

