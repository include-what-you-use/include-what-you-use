//===--- enum.c - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Test that IWYU doesn't suggest to forward-declare C-style enumeration.

#include "tests/c/enum-direct.h"

// IWYU: Enum is...*enum-indirect.h
enum Enum e;

/**** IWYU_SUMMARY

tests/c/enum.c should add these lines:
#include "tests/c/enum-indirect.h"

tests/c/enum.c should remove these lines:
- #include "tests/c/enum-direct.h"  // lines XX-XX

The full include-list for tests/c/enum.c:
#include "tests/c/enum-indirect.h"  // for Enum

***** IWYU_SUMMARY */
