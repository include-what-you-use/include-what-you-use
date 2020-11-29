//===--- range_for.cc - test input file for iwyu --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++11 -I .

// Verify correct handling of the C++11 range-for statement.
// The range-init expression always needs the complete type.
// The loop variable should behave like any variable use.

#include "tests/cxx/range_for-iterable.h"
#include "tests/cxx/direct.h"

int ref_item(const Iterable& items) {
  int total = 0;
  // IWYU: IndirectClass needs a declaration
  for (const IndirectClass& i : items)
    ;
  return 0;
}

int value_item(const Iterable& items) {
  // IWYU: IndirectClass is...*indirect.h
  for (IndirectClass i : items)
    ;
  return 0;
}


/**** IWYU_SUMMARY

tests/cxx/range_for.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/range_for.cc should remove these lines:
- #include "tests/cxx/direct.h"  // lines XX-XX

The full include-list for tests/cxx/range_for.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/range_for-iterable.h"  // for Iterable

***** IWYU_SUMMARY */
