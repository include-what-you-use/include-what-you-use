//===--- builtins_with_mapping.cc - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --mapping_file=tests/cxx/builtins_with_mapping.imp -I .

#include "tests/cxx/builtins_with_mapping.h"
#include "tests/cxx/builtins_with_mapping-d1.h"

// Normally if we use a builtin function IWYU will ignore uses of it.
// However, if there is a mapping defined for that builtin then it should be
// respected.
// Clang considers the definition of a builtin to be at its first use, so we
// have two test cases:

// First test case for a builtin which was already used in a header we included
// IWYU: __builtin_expect is...*builtins_with_mapping-d2.h
int j = __builtin_expect(i, 0);
// Second test case for a first use of a builtin
// IWYU: __builtin_strlen is...*builtins_with_mapping-d3.h
int k = __builtin_strlen("");

/**** IWYU_SUMMARY

tests/cxx/builtins_with_mapping.cc should add these lines:
#include "tests/cxx/builtins_with_mapping-d2.h"
#include "tests/cxx/builtins_with_mapping-d3.h"

tests/cxx/builtins_with_mapping.cc should remove these lines:

The full include-list for tests/cxx/builtins_with_mapping.cc:
#include "tests/cxx/builtins_with_mapping.h"
#include "tests/cxx/builtins_with_mapping-d1.h"  // for i
#include "tests/cxx/builtins_with_mapping-d2.h"  // for __builtin_expect
#include "tests/cxx/builtins_with_mapping-d3.h"  // for __builtin_strlen

***** IWYU_SUMMARY */
