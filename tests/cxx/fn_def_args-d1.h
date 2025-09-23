//===--- fn_def_args-d1.h - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/fn_def_args-i1.h"

struct Struct {
  // IWYU: GetInt is...*fn_def_args-i2.h
  static void FnWithDefArg(int = GetInt());
};

/**** IWYU_SUMMARY

tests/cxx/fn_def_args-d1.h should add these lines:
#include "tests/cxx/fn_def_args-i2.h"

tests/cxx/fn_def_args-d1.h should remove these lines:
- #include "tests/cxx/fn_def_args-i1.h"  // lines XX-XX

The full include-list for tests/cxx/fn_def_args-d1.h:
#include "tests/cxx/fn_def_args-i2.h"  // for GetInt

***** IWYU_SUMMARY */
