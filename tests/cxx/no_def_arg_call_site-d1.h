//===--- no_def_arg_call_site-d1.h - test input file for iwyu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/no_def_arg_call_site-i1.h"

struct Struct {
  // IWYU: GetInt is...*no_def_arg_call_site-i2.h
  static void FnWithDefArg(int = GetInt());
};

/**** IWYU_SUMMARY

tests/cxx/no_def_arg_call_site-d1.h should add these lines:
#include "tests/cxx/no_def_arg_call_site-i2.h"

tests/cxx/no_def_arg_call_site-d1.h should remove these lines:
- #include "tests/cxx/no_def_arg_call_site-i1.h"  // lines XX-XX

The full include-list for tests/cxx/no_def_arg_call_site-d1.h:
#include "tests/cxx/no_def_arg_call_site-i2.h"  // for GetInt

***** IWYU_SUMMARY */
