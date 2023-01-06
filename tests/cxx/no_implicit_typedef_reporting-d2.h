//===--- no_implicit_typedef_reporting-d2.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/no_implicit_typedef_reporting-d1.h"

struct Struct {
  // IWYU: Int is...*no_implicit_typedef_reporting-i1.h
  Int typedefed;
};

/**** IWYU_SUMMARY

tests/cxx/no_implicit_typedef_reporting-d2.h should add these lines:
#include "tests/cxx/no_implicit_typedef_reporting-i1.h"

tests/cxx/no_implicit_typedef_reporting-d2.h should remove these lines:
- #include "tests/cxx/no_implicit_typedef_reporting-d1.h"  // lines XX-XX

The full include-list for tests/cxx/no_implicit_typedef_reporting-d2.h:
#include "tests/cxx/no_implicit_typedef_reporting-i1.h"  // for Int

***** IWYU_SUMMARY */
