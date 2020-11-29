//===--- dotdot.cc - test input file for iwyu -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Tests that IWYU path canonicalization helps understand that
// "tests/cxx/subdir/../indirect.h" and "tests/cxx/indirect.h" are the same
// file.
#include "subdir/dotdot_indirect.h"

// IWYU: IndirectClass is...*indirect.h
IndirectClass x;

/**** IWYU_SUMMARY

tests/cxx/dotdot.cc should add these lines:
#include "tests/cxx/indirect.h"

tests/cxx/dotdot.cc should remove these lines:
- #include "subdir/dotdot_indirect.h"  // lines XX-XX

The full include-list for tests/cxx/dotdot.cc:
#include "tests/cxx/indirect.h"  // for IndirectClass

***** IWYU_SUMMARY */
