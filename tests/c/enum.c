//===--- enum.c - test input file for iwyu --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

// Test that IWYU doesn't suggest to forward-declare C-style enumeration, except
// opaque-declarations of enumerations with fixed underlying type.

#include "tests/c/enum-direct.h"

// IWYU: Enum is...*enum-i1.h
enum Enum e;

// IWYU: EnumFixed needs a declaration
enum EnumFixed ef;
// IWYU: EnumFixed needs a declaration
enum EnumFixed* pef;

/**** IWYU_SUMMARY

tests/c/enum.c should add these lines:
#include "tests/c/enum-i1.h"
enum EnumFixed : int;

tests/c/enum.c should remove these lines:
- #include "tests/c/enum-direct.h"  // lines XX-XX

The full include-list for tests/c/enum.c:
#include "tests/c/enum-i1.h"  // for Enum
enum EnumFixed : int;

***** IWYU_SUMMARY */
