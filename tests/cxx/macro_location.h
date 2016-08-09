//===--- macro_location.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "tests/cxx/indirect.h"
#include "tests/cxx/macro_location-d2.h"
#include "tests/cxx/macro_location-d3.h"
#include "tests/cxx/macro_location-d4.h"

class Foo;
static Foo *foo = 0;

class HClass { };

// IWYU: Foo is...*macro_location-i3.h
const int s = ARRAYSIZE(foo);
// Should not need a declaration of NewClass_Bar, or NewClass.
NEW_CLASS(Bar);
// This shouldn't cause weird iwyu issues between us and macro_location-d2.h.
USE_CLASS(HClass);
// This shouldn't cause macro_location-d1.h to need to include us for HClass.
CREATE_VAR(HClass);
// This should force us to #include a definition of IndirectClass, because it's
// forward-declared by DECLARE_INDIRECT's file.
DECLARE_INDIRECT(global_indirect);

// Macro-concatenated locations end up in <scratch space>, check that they're
// properly attributed to expansion.
#define CONCAT(a, b) a##b

// IWYU: ConcatFwdDeclClass needs a declaration
CONCAT(Concat, FwdDeclClass) *global_concat_ptr;

// IWYU: ConcatClass is...*macro_location-i4.h
CONCAT(Concat, Class) global_concat;

// Make sure types defined and used only within a macro definition file
// aren't attributed to the macro expansion loc.

// IWYU: UNNAMED_TYPE_IN_MACRO is...*macro_location-i5.h
UNNAMED_TYPE_IN_MACRO(500);

void func() {
  LOG_INFO("hello");
  DECLARE_AND_USE_CLASS("hello again");
}

/**** IWYU_SUMMARY

tests/cxx/macro_location.h should add these lines:
#include "tests/cxx/macro_location-i3.h"
#include "tests/cxx/macro_location-i4.h"
#include "tests/cxx/macro_location-i5.h"

tests/cxx/macro_location.h should remove these lines:
- #include "tests/cxx/macro_location-d3.h"  // lines XX-XX
- class Foo;  // lines XX-XX

The full include-list for tests/cxx/macro_location.h:
#include "tests/cxx/indirect.h"  // for IndirectClass
#include "tests/cxx/macro_location-d2.h"  // for ARRAYSIZE, CREATE_VAR, DECLARE_INDIRECT, NEW_CLASS, USE_CLASS
#include "tests/cxx/macro_location-d4.h"  // for DECLARE_AND_USE_CLASS, LOG_INFO
#include "tests/cxx/macro_location-i3.h"  // for Foo
#include "tests/cxx/macro_location-i4.h"  // for ConcatClass, ConcatFwdDeclClass (ptr only)
#include "tests/cxx/macro_location-i5.h"  // for UNNAMED_TYPE_IN_MACRO


***** IWYU_SUMMARY */
