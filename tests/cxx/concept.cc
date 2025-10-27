//===--- concept.cc - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I . --std=c++20

// Tests C++20 concepts usage reporting.

#include "tests/cxx/concept-direct.h"

// IWYU: Concept is...*concept-indirect.h
constexpr bool b = Concept<int>;

// IWYU: Concept is...*concept-indirect.h
Concept auto i = 1;

/**** IWYU_SUMMARY

tests/cxx/concept.cc should add these lines:
#include "tests/cxx/concept-indirect.h"

tests/cxx/concept.cc should remove these lines:
- #include "tests/cxx/concept-direct.h"  // lines XX-XX

The full include-list for tests/cxx/concept.cc:
#include "tests/cxx/concept-indirect.h"  // for Concept

***** IWYU_SUMMARY */
