//===--- std_symbol_mapping.cc - test input file for iwyu -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests the internal IWYU standard library symbol mapping. In particular, tests
// that the mapping works with types in the fwd-decl context.

#include "tests/cxx/std_symbol_mapping-direct.h"

// IWYU: pair is...*<utility>
std::pair<int, int>* p;

template <typename T>
// IWYU: pair is...*<utility>
std::pair<T, T> foo();

/**** IWYU_SUMMARY

tests/cxx/std_symbol_mapping.cc should add these lines:
#include <utility>

tests/cxx/std_symbol_mapping.cc should remove these lines:
- #include "tests/cxx/std_symbol_mapping-direct.h"  // lines XX-XX

The full include-list for tests/cxx/std_symbol_mapping.cc:
#include <utility>  // for pair

***** IWYU_SUMMARY */
