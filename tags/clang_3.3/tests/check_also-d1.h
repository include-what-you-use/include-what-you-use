//===--- check_also-d1.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the '--check_also' flag.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_CHECK_ALSO_D1_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_CHECK_ALSO_D1_H_

#include "check_also-i1.h"

// IWYU: NULL is...*<stddef.h>
int* unused = NULL;   // NULL comes from check_also-i1.h

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_TESTS_CHECK_ALSO_D1_H_

/**** IWYU_SUMMARY

tests/check_also-d1.h should add these lines:
#include <stddef.h>

tests/check_also-d1.h should remove these lines:
- #include "check_also-i1.h"  // lines XX-XX

The full include-list for tests/check_also-d1.h:
#include <stddef.h>  // for NULL

***** IWYU_SUMMARY */
