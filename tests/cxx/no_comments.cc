//===--- no_comments.cc - test input file for iwyu ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -Xiwyu --no_comments -I .

// Test that passing the --no_comments switch to IWYU suppresses both
// '// lines NN-NN' and '// for symbol' comments.

#include "tests/cxx/direct.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass global;

/**** IWYU_SUMMARY

tests/cxx/no_comments.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/no_comments.cc should remove these lines:
- #include "tests/cxx/direct.h"

The full include-list for tests/cxx/no_comments.cc:
#include "tests/cxx/indirect.h"

***** IWYU_SUMMARY */
