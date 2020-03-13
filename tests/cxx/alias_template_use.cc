//===--- alias_template_use.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests that use of template aliases is assigned to the header defining the
// alias, rather than the underlying type.

#include "alias_template_use-d1.h"

template<typename T>
class A {
};

class B {
  // IWYU: AliasTemplate is...*alias_template_use-i1.h
  A<AliasTemplate<int>> a;
};

/**** IWYU_SUMMARY

tests/cxx/alias_template_use.cc should add these lines:
#include "alias_template_use-i1.h"

tests/cxx/alias_template_use.cc should remove these lines:
- #include "alias_template_use-d1.h"  // lines XX-XX

The full include-list for tests/cxx/alias_template_use.cc:
#include "alias_template_use-i1.h"  // for AliasTemplate

***** IWYU_SUMMARY */
