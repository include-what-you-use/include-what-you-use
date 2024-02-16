//===--- var_def_is_use.c - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -I .

#include "var_def_is_use-d1.h"

// IWYU: extern_global is...*var_def_is_use-i1.h
// IWYU: extern_global is...*var_def_is_use-i2.h
int extern_global = 20;

/**** IWYU_SUMMARY

tests/c/var_def_is_use.c should add these lines:
#include "tests/c/var_def_is_use-i1.h"
#include "tests/c/var_def_is_use-i2.h"

tests/c/var_def_is_use.c should remove these lines:
- #include "var_def_is_use-d1.h"  // lines XX-XX

The full include-list for tests/c/var_def_is_use.c:
#include "tests/c/var_def_is_use-i1.h"  // for extern_global
#include "tests/c/var_def_is_use-i2.h"  // for extern_global

***** IWYU_SUMMARY */
