//===--- macro_location.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/macro_location-d2.h"
#include "tests/cxx/macro_location-d3.h"

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


/**** IWYU_SUMMARY

tests/cxx/macro_location.h should add these lines:
#include "tests/cxx/macro_location-i3.h"

tests/cxx/macro_location.h should remove these lines:
- #include "tests/cxx/macro_location-d3.h"  // lines XX-XX
- class Foo;  // lines XX-XX

The full include-list for tests/cxx/macro_location.h:
#include "tests/cxx/macro_location-d2.h"  // for ARRAYSIZE, CREATE_VAR, NEW_CLASS, USE_CLASS
#include "tests/cxx/macro_location-i3.h"  // for Foo


***** IWYU_SUMMARY */
