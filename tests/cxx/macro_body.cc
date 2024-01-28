//===--- macro_body.cc - test input file for iwyu -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test IWYU's analysis of macro bodies.

#include "tests/cxx/macro_body-i1.h"
#include "tests/cxx/macro_body-i2.h"

// IWYU_ARGS: -I .

// IWYU: OBJECT_LIKE is...*macro_body-d1.h
#define MACRO1(x) OBJECT_LIKE

// IWYU: FUNCTION_LIKE is...*macro_body-d2.h
#define MACRO2(x) FUNCTION_LIKE(x)

// FUNCTION_LIKE is not a known macro without the arguments.
// UNDEFINED is not defined, and so will not be accounted.
#define MACRO3(x) FUNCTION_LIKE UNDEFINED

// Builtin macros do not have a location, and are not reported.
#define MACRO4(x) __FILE__

/**** IWYU_SUMMARY

tests/cxx/macro_body.cc should add these lines:
#include "tests/cxx/macro_body-d1.h"
#include "tests/cxx/macro_body-d2.h"

tests/cxx/macro_body.cc should remove these lines:
- #include "tests/cxx/macro_body-i1.h"  // lines XX-XX
- #include "tests/cxx/macro_body-i2.h"  // lines XX-XX

The full include-list for tests/cxx/macro_body.cc:
#include "tests/cxx/macro_body-d1.h"  // for OBJECT_LIKE
#include "tests/cxx/macro_body-d2.h"  // for FUNCTION_LIKE

***** IWYU_SUMMARY */
