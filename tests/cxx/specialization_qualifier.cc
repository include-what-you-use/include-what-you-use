//===--- specialization_qualifier.cc - test input -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . -std=c++11

#include "tests/cxx/direct.h"
#include "tests/cxx/specialization_qualifier-direct.h"

// Instantiating Box to resolve Nested requires its by-value member's type.
// The same Box node is first an argument, then a nested-template qualifier.
// IWYU: Box is...*specialization_qualifier-inst.h
// IWYU: Holder is...*specialization_qualifier-indirect.h
// IWYU: IndirectClass is...*indirect.h
// IWYU: IndirectClass needs a declaration
Holder<IndirectClass> instance;

/**** IWYU_SUMMARY

tests/cxx/specialization_qualifier.cc should add these lines:
#include "tests/cxx/indirect.h"
#include "tests/cxx/specialization_qualifier-indirect.h"
#include "tests/cxx/specialization_qualifier-inst.h"

tests/cxx/specialization_qualifier.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX
- #include "tests/cxx/specialization_qualifier-direct.h"  // lines XX-XX

The full include-list for tests/cxx/specialization_qualifier.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/specialization_qualifier-indirect.h"  // for Holder
#include "tests/cxx/specialization_qualifier-inst.h"  // for Box

***** IWYU_SUMMARY */
